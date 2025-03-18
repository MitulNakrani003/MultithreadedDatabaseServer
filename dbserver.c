#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "proj2.h"

// Change these values as required
#define DEFAULT_PORT "5000"
#define LISTENER_QUEUE_SIZE 2
#define MAX_BUFF_LEN 1024

int listener()
{
	int conn_sock = -1;

	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_PASSIVE};

	struct addrinfo *listenerinfo = {0};
	if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &listenerinfo) < 0)
	{
		perror("getaddrinfo");
		return 1;
	}

	struct addrinfo *curr = NULL;
	int listener_sock = -1;
	for (curr = listenerinfo; curr != NULL; curr = curr->ai_next)
	{
		listener_sock = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
		if (listener_sock == -1)
		{
			perror("listener: socket");
			continue;
		}

		if (bind(listener_sock, curr->ai_addr, curr->ai_addrlen) < 0)
		{
			close(listener_sock);
			perror("listener: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(listenerinfo);

	if (curr == NULL)
	{
		fprintf(stderr, "listner: failed to bind\n");
		exit(1);
	}

	if (listen(listener_sock, LISTENER_QUEUE_SIZE) == -1)
	{
		perror("listen");
		exit(1); 
	}

	printf("Waiting for connections... \n");

	while (1)
	{
		conn_sock = accept(listener_sock, NULL, NULL);
		if (conn_sock == -1)
		{
			perror("connection: accept");
			continue;
		}

		printf("Client connected:\n");

		char msg[MAX_BUFF_LEN] = {'\0'};
		int bytes_recvd = recv(conn_sock, msg, MAX_BUFF_LEN - 1, 0);

		struct request req = {0};
		req.op_status = msg[0];
		strncpy(req.name, msg + 1, 31);
		strncpy(req.len, msg + 32, 8);
		char data[4096] = {'\0'};
		strncpy(data, msg + 40, 4096);
			
		if (bytes_recvd > 0) {
			printf("Operation: %c\n", req.op_status);
			printf("Name: %s\n", req.name);
			printf("Length: %s\n", req.len);
			printf("Data: %s\n", data);
		}

		if (send(conn_sock, msg, sizeof(msg), 0) < 0) {
			perror("send");
		}
		close(conn_sock);
	}
	close(listener_sock);
	return 0;
}

int main(int argc, char **argv)
{
	int status = listener();
	if (status != 0)
	{
		return 1;
	}
	return 0;
}
