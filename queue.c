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
		return -1;
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

int isempty(queue_t *queue) {
	if (queue == NULL || queue->front == NULL) {
		return 0;	
	}
	return 1;
}

int get_queue_size(queue_t *queue) {
    if (queue == NULL) {
        return 0;
    }
    int size = 0;
    node_t *current = queue->front;
    while (current != NULL) {
        size++;
        current = current->next;
    }
    return size;
}