#include <stdlib.h>
#include "queue.h"

queue_t *create_queue()
{
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    queue->front = NULL;
    queue->rear = NULL;
    return queue;
}

void enqueue(queue_t *queue, int sock_fd)
{
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->sock_fd = sock_fd;
    new_node->next = NULL;
    if (queue->rear){
        queue->rear->next = new_node;
    }
    else{
        queue->front = new_node;
    }
    queue->rear = new_node;
}

int dequeue(queue_t *queue)
{
    if (queue->front == NULL){
        return NULL;
    }
    node_t *temp = queue->front;
    int sock_fd = temp->sock_fd;
    queue->front = temp->next;
    if (queue->front == NULL){
        queue->rear = NULL;
    }
    free(temp);
    return sock_fd;
}
