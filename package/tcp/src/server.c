#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>

#define SERVPORT 3333
#define BACKLOG 10
#define MAX_CONNECTED_NO 10
#define MAXDATASIZE (5*1024)

int main()
{
	struct sockaddr_in server_sockaddr,client_sockaddr;
	int sin_size,recvbytes,sendbytes;
	int sockfd,client_fd;
	char buf[MAXDATASIZE];

	if((sockfd = socket(AF_INET, SOCK_STREAM,0)) == -1)
	{
		perror("socket:");
		exit(1);
	}

	int on;
	on = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port = htons(SERVPORT);
	server_sockaddr.sin_addr.s_addr=INADDR_ANY;
	memset(&(server_sockaddr.sin_zero),0,8);

	if((bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))) == -1)	
	{
		perror("bind:");
		exit(1);
	}

	if(listen(sockfd,BACKLOG) == -1)
	{
		perror("listen:");
		exit(1);
	}
	printf("Start listen.....\n");

	sin_size = sizeof(struct sockaddr);
	if((client_fd = accept(sockfd,(struct sockaddr *)&client_sockaddr,&sin_size)) == -1)
	{
		perror("accept:");
		exit(1);
	}

	struct sockaddr_in peeraddr;
	int len = sizeof(peeraddr);
	char server_ip[20];
	printf("accept socket..... Client address: %s Port: %d\n",\
			inet_ntoa(client_sockaddr.sin_addr),ntohs(client_sockaddr.sin_port));	
	getsockname(client_fd,(struct sockaddr *)&peeraddr,&len);
	inet_ntop(AF_INET, &peeraddr.sin_addr,server_ip,sizeof(server_ip));
	printf("accept socket..... Server address: %s\n",server_ip);	

	if((recvbytes=recv(client_fd,buf,MAXDATASIZE,0)) == -1)
	{
		perror("recv:");	
		exit(1);
	}
	printf("Recv bytes: %d, buf: 0x%x\n",recvbytes, buf[2]);

	if((sendbytes = send(client_fd,buf,sizeof(buf),0)) == -1)
	{
		perror("send:");
		exit(1);
	}
	printf("Send butes: %d \n",sendbytes);
	close(sockfd);
}
