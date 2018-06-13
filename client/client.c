#include <unistd.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <wait.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include "error.h"


/*进程共享文件用于统计创建进程个数*/
#define PFILE_NAME	"count"


struct shared {
	sem_t mutex;	/*信号量用于加锁*/
	int count;		/*进程个数*/
} shared;


void request(const char *server_ip, int server_port)
{
	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(0);
	
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(client_socket < 0) sys_exit_throw("create client socket fail");
	
	struct sockaddr_in server_addr;
	bzero((char *)&server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;

	struct hostent *server = gethostbyname(server_ip);
	if(!server) sys_exit_throw("fail to get host name");

	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);

	server_addr.sin_port = htons(server_port);
	socklen_t server_addr_len = sizeof(server_addr);

	if(connect(client_socket, (struct sockaddr*) &server_addr, server_addr_len) == -1 ) {
		sys_exit_throw("connent to server fail");
	}

	int pid = getpid();

	char content[64] = {0};
	sprintf(content, "%s, pid:%d", "i am client！", pid);

	send(client_socket, content, strlen(content), 0);

	close(client_socket);
}

/*
  参数1为serverip，参数2为server端口号
*/
int main(int argc,char *argv[])
{
	if(argc != 3) exit_throw("parameter error!\n");

	char *server_ip = argv[1];
	int server_port = atoi(argv[2]);

	struct shared *psh;

	/*创建共享文件*/
	int fd = open(PFILE_NAME, O_RDWR | O_CREAT, 0666);
	/*初始化0*/
	write(fd, &shared, sizeof(struct shared));
	/*映射内存*/
	psh = mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	sem_init(&psh->mutex, 1, 1);

	/*初始进程个数+1*/
	psh->count++;
	
	int i, status;
	for (i = 0; i < 10; i++) {
		pid_t fpid = fork();
		if (0 == fpid) {
			sem_wait(&psh->mutex);
			psh->count++;
			printf("%d processes was created!\n", psh->count);
			sem_post(&psh->mutex);
			request(server_ip, server_port);
		}
		else if (fpid > 0) {
			request(server_ip, server_port);
			wait(&status);
		}
		else
			sys_exit_throw("fork error!");
	}

	return 0;
}