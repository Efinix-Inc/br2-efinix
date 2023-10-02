/*
 * Copyright (C) 2023 Efinix Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

void help()
{
	printf("help\n");
	printf("-a\t\tserver IP address\n");
	printf("-c\t\tclient mode\n");
	printf("-m\t\tmessage to send to server\n");
	printf("-s\t\tserver mode\n");
	printf("-p\t\tport number. Default is 9123\n");
	printf("-u\t\tuse UDP packet instead of TCP packet\n");
	printf("-h\t\tshow this help message\n");
	printf("\nExample usage\n");
	printf("server,\n\t\t ./socket_hello_world -s\n");
	printf("client,\n\t\t ./socket_hello_world -c -a <server ip>\n");
}

int client_mode(char *ipaddr, int port, int proto, char *msg)
{
	int client_fd;
	int status;
	struct sockaddr_in serv_addr;
	int addrlen = sizeof(serv_addr);
	char buffer[1024] = {0};
	int sock_type = SOCK_STREAM;

	if (proto == 1)
		sock_type = SOCK_DGRAM;

	if ((client_fd = socket(AF_INET, sock_type, 0)) < 0) {
		printf("Failed to create a socket\n");
		return -1;	
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// convert IPv4 and IPv6 addresses from text to binary
	if (inet_pton(AF_INET, ipaddr, &serv_addr.sin_addr) < 0) {
		printf("Invalid address %s\n", ipaddr);
		return -1;
	}

	if ((connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
		printf("Connection failed\n");
		return -1;
	}

	if (proto == 1) {
		sendto(client_fd, msg, 1000, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

		recvfrom(client_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&serv_addr, (socklen_t *)&addrlen); //sizeof(serv_addr));
		printf("server: %s\n", buffer);

	} else { // TCP
		send(client_fd, msg, strlen(msg), 0);
		printf("client sent msg\n");

		status = read(client_fd, buffer, sizeof(buffer));
		printf("server: %s\n", buffer);
	}

	close(client_fd);

	return 0;
}

int server_mode(int port, int proto)
{
	int server_fd, new_socket;
	struct sockaddr_in serv_addr, cli_addr;
	int addrlen = sizeof(serv_addr);
	int clilen = sizeof(cli_addr);
	int sock_type = SOCK_STREAM;
	int status, n;
	char buffer[1024] = {0};
	char *msg = "Hello from server";

	if (proto == 1)
		sock_type = SOCK_DGRAM;

	if ((server_fd = socket(AF_INET, sock_type, 0)) < 0) {
		printf("Failed to create a socket\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr *)&serv_addr, addrlen) < 0) {
		printf("Failed to bind the port %d\n", port);
		return -1;
	}

	if (proto == 1) { // UDP
		n = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cli_addr, &clilen);
		buffer[n] = '\0';
		printf("client: %s\n", buffer);

		// send the response
		sendto(server_fd, msg, 1000, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		printf("server: UDP message sent\n");

	} else { // TCP
		if (listen(server_fd, 3) < 0) {
			printf("Failed to listen to port %d\n", port);
			return -1;
		}

		if ((new_socket = accept(server_fd, (struct sockaddr *)&serv_addr, (socklen_t *)&addrlen)) < 0) {
			printf("Failed to create a new socket\n");
			return -1;		
		}

		status = read(new_socket, buffer, sizeof(buffer));
		printf("client: %s\n", buffer);

		send(new_socket, msg, strlen(msg), 0);
		printf("server: TCP message sent\n");

		close(new_socket);
	}

	shutdown(server_fd, SHUT_RDWR);
	//close(server_fd);

	return 0;
}

int main(int argc, char * const* argv)
{
	int cmd_opt = 0;
	char ipaddr[50] = {'\0'};
	int port = 9123;
	int ret;
	int mode = 0;
	int proto = 0;
	char msg[1024] = "hello from client";

	while (cmd_opt != -1) {
		cmd_opt = getopt(argc, argv, "a:m:p:csuh");

		switch(cmd_opt) {
			case 'a':
				sprintf(ipaddr, "%s", optarg);
				break;

			case 'c':
				mode = 1;
				break;

			case 'm':
				sprintf(msg, "%s", optarg);
				break;

			case 's':
				mode = 2;
				break;

			case 'p':
				port = atoi(optarg);
				break;

			case 'u':
				proto = 1;
				break;

			case 'h':
				help();
				return 0;

			case -1:
				break;

			default:
				help();
				return 1;
		}
	}

	printf("port = %d\n", port);

	if (proto == 1)
		printf("protocol: UDP\n");
	else
		printf("protocol: TCP\n");

	printf("msg = %s\n", msg);

	switch(mode) {
	case 1:
		printf("mode = client\n");
		if (ipaddr[0] == '\0') {
			printf("Error: IP address is not set\n");
			help();
			return -1;
		}
		printf("ipaddr = %s\n", ipaddr);
		ret = client_mode(ipaddr, port, proto, msg);
		if (ret)
			return -1;

		break;

	case 2:
		printf("mode = server\n");
		ret = server_mode(port, proto);
		if (ret)
			return -1;

		break;

	default:
		printf("Error: set mode either client or server\n");
		help();
		return -1;
	}

	return 0;
}

