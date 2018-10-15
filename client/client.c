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

/*需要创建的进程数*/
#define PROCESS_NUM		100

/*每个进程请求次数*/
#define REQUEST_TIMES	10000


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
	if(client_socket < 0) exit_throw("create client socket fail");
	
	struct sockaddr_in server_addr;
	bzero((char *)&server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;

	struct hostent *server = gethostbyname(server_ip);
	if(!server) exit_throw("fail to get host name");

	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);

	server_addr.sin_port = htons(server_port);
	socklen_t server_addr_len = sizeof(server_addr);

	if(connect(client_socket, (struct sockaddr*) &server_addr, server_addr_len) == -1 ) {
		exit_throw("connent to server fail");
	}

	int pid = getpid();

	char content[64] = {0};
	sprintf(content, "%s, pid:%d\n", "i am client！", pid);
	for (int i = 0; i < REQUEST_TIMES; ++i) {
		send(client_socket, content, strlen(content), 0);
		usleep(10000);	//10ms
	}

	close(client_socket);

	exit(0);
}

/*
 * 参数1为serverip，参数2为server端口号
 */
int main(int argc,char *argv[])
{
	if(argc != 3) exit_throw("parameter error!\n");

	char *server_ip = argv[1];
	int server_port = atoi(argv[2]);

	struct shared *psh;

	/*创建共享文件*/
	int fd = open(PFILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
	/*初始化0*/
	int ret_len = write(fd, &shared, sizeof(struct shared));
	if (ret_len != sizeof(struct shared)) {
		exit_throw("write error!\n");
	}
	/*映射内存*/
	psh = mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	sem_init(&psh->mutex, 1, 1);
	
	int i, pid[PROCESS_NUM];
	for (i = 0; i < PROCESS_NUM; i++) {
		pid_t fpid = fork();
		if (0 == fpid) {
			pid[i]=getpid();
			sem_wait(&psh->mutex);
			psh->count++;
			printf("%d processes was created!\n", psh->count);
			sem_post(&psh->mutex);
			request(server_ip, server_port);
		}
		else if (fpid > 0) {
		}
		else {
			exit_throw("fork error!");
		}
	}

	/*等待所有子进程创建完毕*/
	while (psh->count < PROCESS_NUM) {
		sleep(0);
	}

	wait(NULL);

	remove(PFILE_NAME);
	
	printf("exit all!\n");

	return 0;
}