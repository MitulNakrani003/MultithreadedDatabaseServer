#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "proj2.h"
#include "queue.h"
#include "dboperations.h"
#include "dbservices.h"

pthread_t listener_thread;
pthread_t worker_threads[MAX_WORKERS];

int main(int argc, char **argv)
{
	char *port = DEFAULT_PORT;
	if (argc >= 2) {
		port = argv[1];
	}

	cleanup_resources(); // deletes data files in the base folder

	db_init(); // initialize database

	work_queue = create_queue(); // worker queue

	char *port_copy = strdup(port);
	pthread_create(&listener_thread, NULL, listener, port_copy);
	for (int i = 0; i < MAX_WORKERS; i++) {
		pthread_create(&worker_threads[i], NULL, distribute_worker, NULL);
	}

	console_handler(); // handle command line inputs (e.g., stats, quit)

	return 0;
}
