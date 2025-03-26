#ifndef DBSERVICES_H
#define DBSERVICES_H

#include <pthread.h>
#include <signal.h>

#include "queue.h"

#define DEFAULT_PORT "5000"
#define MAX_WORKERS 4

// Function declarations
void queue_work(int sock_fd);
int get_work();
void* listener(void *arg);
void handle_work(int sock_fd);
void* distribute_worker();
void update_stats(char op, int status);
void console_handler();
void cleanup_resources();

extern queue_t *work_queue; // Externally declared queue
extern volatile sig_atomic_t running; 
extern pthread_t listener_thread;
extern pthread_t worker_threads[MAX_WORKERS];

#endif
