#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include "proj2.h"
#include "dbservices.h"
#include "queue.h"
#include "dboperations.h"

pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for condition variable
pthread_cond_t queue_fill = PTHREAD_COND_INITIALIZER; // Non-empty queue condition
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for statistics

// Queue Instance
queue_t *work_queue = NULL;

// Global variables to keep track of statistics
int total_requests = 0;
int write_count = 0;
int read_count = 0;
int delete_count = 0;
int failed_count = 0;

#define LISTENER_QUEUE_SIZE 2

void queue_work(int sock_fd) {
	pthread_mutex_lock(&cond_mutex);
	enqueue(work_queue, sock_fd);
	pthread_cond_signal(&queue_fill);
	pthread_mutex_unlock(&cond_mutex);
}

int get_work() {
	pthread_mutex_lock(&cond_mutex);
	while (isempty(work_queue)) {
		pthread_cond_wait(&queue_fill, &cond_mutex);
	}
	int sock_fd = dequeue(work_queue);
	pthread_mutex_unlock(&cond_mutex);
	return sock_fd;
}

void* listener(void *arg)
{
	char *port = (char *)arg;
	int conn_sock = -1;

	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_PASSIVE};

	struct addrinfo *listenerinfo = {0};
	if (getaddrinfo(NULL, port, &hints, &listenerinfo) < 0)
	{
		perror("getaddrinfo");
		return NULL;
	}

	struct addrinfo *curr = NULL;
	int listener_sock = -1;
	for (curr = listenerinfo; curr != NULL; curr = curr->ai_next)
	{
		listener_sock = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
		if (listener_sock == -1)
		{
			free(port);
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
		printf("Connection accepted on socket %d\n.", conn_sock); //Test Print

		// Enqueue the work
		queue_work(conn_sock);

		// Below Code for Testing
		// int sock_fd = get_work();
		// printf("Got work from queue: %d\n", sock_fd); // Test Print
		// handle_work(sock_fd);
	}
	free(port);
	close(listener_sock);
	return NULL;
}

void handle_work(int sock_fd)
{
	struct request req = {0};
	char data[DB_VALUE_MAXLENGTH + 1] = "";
	ssize_t bytes_recvd = 0;
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
	int status = -1;
	int error = 0; // flag to check if an error occurred in db operation
	struct request res = {0}; // response header
	strcpy(res.name, ""); // name is irrelevant
	res.op_status = 'X'; // Fail status
			     // printf("Received request: %s\n", req.name); // Test Print
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
						error = 1;
						break;
					}
					total_bytes += bytes_recvd;
				}
				// printf("Received data: %s\n", data); // Test Print
				status = db_write(req.name, data);
				update_stats('W', status);	
				if (status < 0) {
					perror("write unsuccessful");
					error = 1;	
					break;
				}
				res.op_status = 'K'; // write successful
				send(sock_fd, (void *)&res, sizeof(res), 0); // send response header
			}
			break;
		case 'R':
			status = db_read(req.name, data);
			update_stats('R', status);
			if (status < 0) {
				perror("read unsuccessful");
				error = 1;
				break;
			}
			res.op_status = 'K'; // read successful
			sprintf(res.len, "%ld", strlen(data)); // set data length
			send(sock_fd, (void *)&res, sizeof(res), 0); // send response header
			send(sock_fd, data, strlen(data), 0); // send data
			break;
		case 'D':
			status = db_delete(req.name);
			update_stats('D', status);
			if (status < 0) {
				perror("delete unsuccessful");
				error = 1;
				break;
			}
			res.op_status = 'K'; // delete successful
			send(sock_fd, (void *)&res, sizeof(res), 0); // send response header
			printf("delete successful!\n");		
			break;

		default:
			break;
	}
	if (error == 1) { // error occurred in the db operation
		send(sock_fd, (void *)&res, sizeof(res), 0); // send error header	 
	}
	close(sock_fd);	// close the current connection
}

void* distribute_worker() {
	while (1) {
		int sock_fd = get_work();
		handle_work(sock_fd);
	}
	return NULL;
}

void update_stats(char op, int status) {
	pthread_mutex_lock(&stats_mutex);
	total_requests++;		
	if (status < 0) 
	{
		failed_count++;
	}
	else
	{
		switch (op) 
		{
			case 'W': 
				write_count++; 
				break;
			case 'R': 
				read_count++; 
				break;
			case 'D': 
				delete_count++; 
				break;
		}
	}
	pthread_mutex_unlock(&stats_mutex);
}

void console_handler() {
	char cmd[128];
	while (fgets(cmd, sizeof(cmd), stdin)) {
		printf("--------------------------------------------------");
		if (strncmp(cmd, "stats\n", 6) == 0) {
			pthread_mutex_lock(&stats_mutex);
			printf("Total requests: %d\n", total_requests);
			printf("Writes: %d\nReads: %d\nDeletes: %d\n", write_count, read_count, delete_count);
			printf("Failed requests: %d\n", failed_count);
			printf("Queued requests: %d\n", get_queue_size(work_queue));
			pthread_mutex_unlock(&stats_mutex);
		} else if (strncmp(cmd, "quit\n", 5) == 0) {
			printf("Shutting down server...\n");
			exit(0);
		}
		printf("--------------------------------------------------");
	}
}

void cleanup_resources() {
	char command[256];
	snprintf(command, sizeof(command), "rm -f %s/data.*", BASE_FOLDER);
	system(command);
}
