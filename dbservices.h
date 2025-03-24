#ifndef DBSERVICES_H
#define DBSERVICES_H

#include "queue.h"

// Function declarations
void queue_work(int sock_fd);
int get_work();
int listener();
void handle_work(int sock_fd);
void* distribute_worker();
void update_stats(char op, int status);
void console_handler();
void cleanup_resources();

extern queue_t *work_queue; // Externally declared queue

#endif
