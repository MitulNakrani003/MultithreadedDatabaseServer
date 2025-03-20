#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "proj2.h"
#include "queue.h"
#include "dboperations.h"

//Queue Instance
queue_t *work_queue = NULL;

// Change these values as required
#define DEFAULT_PORT "5000"
#define LISTENER_QUEUE_SIZE 2

void queue_work(int sock_fd) {
    enqueue(work_queue, sock_fd);
}

int get_work() {
    int sock_fd = dequeue(work_queue);
    return sock_fd;
}


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

		// Testing Handle the request
		// Enqueue the work
	        queue_work(conn_sock);
        	// Dequeue and handle the work
       		int sock = get_work();
        	handle_work(sock);

		// char msg[MAX_BUFF_LEN] = {'\0'};
		// int bytes_recvd = recv(conn_sock, msg, MAX_BUFF_LEN - 1, 0);

		// struct request req = {0};
		// req.op_status = msg[0];
		// strncpy(req.name, msg + 1, 31);
		// strncpy(req.len, msg + 32, 8);
		// char data[4096] = {'\0'};
		// strncpy(data, msg + 40, 4096);
			
		// if (bytes_recvd > 0) {
		// 	printf("Operation: %c\n", req.op_status);
		// 	printf("Name: %s\n", req.name);
		// 	printf("Length: %s\n", req.len);
		// 	printf("Data: %s\n", data);
		// }

		// if (send(conn_sock, msg, sizeof(msg), 0) < 0) {
		// 	perror("send");
		// }
		// close(conn_sock);
	}
	close(listener_sock);
	return 0;
}


void handle_work(int sock_fd)
{
	if (sock_fd == -1)
	{
		perror("connection: accept");
		return;
	}
	printf("Client connected:\n");

	struct request req;
	char data[4096];
    	ssize_t bytes_recvd;
	int total_bytes_recvd = 0;

	while (total_bytes_recvd < sizeof(req)) {
		bytes_recvd = recv(sock_fd, ((char*)&req) + total_bytes_recvd, sizeof(req) - total_bytes_recvd, 0);
	
		if (bytes_recvd <= 0) {
			perror("Failed to read request header");
			close(sock_fd);
			return;
		}
	
		total_bytes_recvd += bytes_recvd;
	}

	int len = atoi(req.len);

	printf("Operation: %c\n", req.op_status);
	printf("Name: %s\n", req.name);
	printf("Length: %s\n", req.len);
	

	switch (req.op_status) {
		case 'W':
			int total_bytes = 0;
			if (len > 0 && len <= 4096) 
			{
				total_bytes = 0;
				while (total_bytes < len) {
					bytes_recvd = recv(sock_fd, data + total_bytes, len - total_bytes, 0);
			
					if (bytes_recvd <= 0) 
					{
						perror("recv failed");
						close(sock_fd);
						return;
					}
					total_bytes += bytes_recvd;
				}
				printf("Data: %s\n", data);
				int status = db_write(req.name, data);
				if (status < 0) {
					perror("write unsuccessful");
					break;
				}
				printf("write successful\n");
			}
			break;
		case 'R':
			
			break;
		case 'D':
			
			break;
		default:
			
			break;
	}
	close(sock_fd);	
}

int main(int argc, char **argv)
{
	work_queue = create_queue();
	int status = listener();
	if (status != 0)
	{
		return 1;
	}
	return 0;
}
