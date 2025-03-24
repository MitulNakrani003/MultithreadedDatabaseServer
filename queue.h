#ifndef QUEUE_H
#define QUEUE_H

typedef struct node
{
    int sock_fd;
    struct node *next;
} node_t;

typedef struct queue
{
    node_t *front;
    node_t *rear;
} queue_t;

queue_t *create_queue();
void enqueue(queue_t *queue, int sock_fd);
int dequeue(queue_t *queue);
int isempty(queue_t *queue);
int get_queue_size(queue_t *queue);
 
#endif
