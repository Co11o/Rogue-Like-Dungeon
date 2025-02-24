#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#define WIDTH 80
#define HEIGHT 21
#define MAX 500


typedef struct queue_node{
    int x_location;//Grid x coordinate
    int y_location;//Grid y coordinate
    int priority;//Current Min Distance 
}queue_node;

typedef struct {
    queue_node queue[MAX];//nodes in queue
    int size;//Number of queue_nodes in queue
} PriorityQueue;

void swap(queue_node* a, queue_node* b);
void heapifyUp(PriorityQueue* pq, int index);
int addNode(PriorityQueue* pq, int x, int y, int priority);
void heapifyDown(PriorityQueue* pq, int index);
int extractNode(PriorityQueue* pq, queue_node* minNode);
int peek(PriorityQueue* pq, queue_node* minNode);
void decrease_priority(PriorityQueue* pq, int x, int y, int new_priority);

#endif