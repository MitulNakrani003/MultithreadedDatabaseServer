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
#include "dbservices.h"

#define MAX_WORKERS 4

int main(int argc, char **argv)
{
	char *port = DEFAULT_PORT;
    if (argc >= 2) {
        port = argv[1];
    }

	cleanup_resources(); // Deletes Data files in the base folder
	
	db_init();

	work_queue = create_queue();

	pthread_t listener_thread, worker_threads[MAX_WORKERS];
	char *port_copy = strdup(port);
	pthread_create(&listener_thread, NULL, listener, port_copy);
	for (int i = 0; i < MAX_WORKERS; i++) {
		pthread_create(&worker_threads[i], NULL, distribute_worker, NULL);
	}

	console_handler();

	
	// listener();
	// int status = listener();
	// if (status != 0)
	// {
	// 	return 1;
	// }
	return 0;
}
