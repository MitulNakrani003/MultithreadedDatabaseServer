#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>


#include "proj2.h"
#include "queue.h"
#include "dboperations.h"

pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex for condition variable
pthread_cond_t queue_fill = PTHREAD_COND_INITIALIZER; // non-empty queue condition

//Queue Instance
queue_t *work_queue = NULL;

// Change these values as required
#define DEFAULT_PORT "5000"
#define LISTENER_QUEUE_SIZE 2
#define MAX_WORKERS 4

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
		printf("Connection accepted on socket %d\n.", conn_sock); //Test Print

		// Enqueue the work
		pthread_mutex_lock(&cond_mutex);
		queue_work(conn_sock);
		pthread_cond_signal(&queue_fill);
		pthread_mutex_unlock(&cond_mutex);
	}
	close(listener_sock);
	return 0;
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
				status = db_write(req.name, data);
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
		pthread_mutex_lock(&cond_mutex);
		while (isempty(&work_queue)) {
			pthread_cond_wait(&queue_fill, &cond_mutex);
		}
		int sock_fd = get_work();
		pthread_mutex_unlock(&cond_mutex);
		handle_work(sock_fd);
	}
	return NULL;
}

void console_handler() {
	char cmd[100];
	while (fgets(cmd, sizeof(cmd), stdin)) {
		if (strncmp(cmd, "stats\n", 6) == 0) {
			printf("Server statistics:\n");
		} else if (strncmp(cmd, "quit\n", 5) == 0) {
			printf("Shutting down server...\n");
			exit(0);
		}
	}
}

void cleanup_resources() {
	char command[256];
	snprintf(command, sizeof(command), "rm -f %s/data.*", BASE_FOLDER);
	system(command);
}

int main(int argc, char **argv)
{
	cleanup_resources();

	pthread_t listener_thread, worker_threads[MAX_WORKERS];
	pthread_create(&listener_thread, NULL, listener, NULL);
	for (int i = 0; i < MAX_WORKERS; i++) {
		pthread_create(&worker_threads[i], NULL, distribute_worker, NULL);
	}

	console_handler();

	work_queue = create_queue();
	int status = listener();
	if (status != 0)
	{
		return 1;
	}
	return 0;
}
