#include<stdio.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<netdb.h>
#include<stdlib.h>
#include "error.h"


int main(int argc,char *argv[])
{
	if (argc != 3) {
		_error_display("parameter error!\n");
		return 0;
	}
	char *server_ip = argv[1];
	int server_port = atoi(argv[2]);
	
	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = INADDR_ANY;
	client_addr.sin_port = htons(0);
	
	//create a socket
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(client_socket < 0) {
		_exit_throw("create client socket fail");
	}
	
	struct sockaddr_in server_addr;
	bzero((char *)&server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;

	struct hostent *server = gethostbyname(server_ip);
	if(!server) {
		_exit_throw("fail to get host name");
	}
	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);

	server_addr.sin_port = htons(server_port);
	socklen_t server_addr_len = sizeof(server_addr);

	if(connect(client_socket, (struct sockaddr*) &server_addr, server_addr_len) == -1 ) {
		_exit_throw("connent to server fail");
	}

	char* content = "i am client";
	send(client_socket, content, strlen(content), 0);


	printf("send completed, size = %d\n", (int)strlen(content));

	close(client_socket);
	

	return 0;
}