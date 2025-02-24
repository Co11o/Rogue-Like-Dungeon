#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <stdint.h>
#include "dungeon_generation.h"
#include "priority_queue.h"
#include <limits.h>

#define WIDTH 80
#define HEIGHT 21
#define MAX 500

// Swap function to swap two queue_nodes
void swap(queue_node* a, queue_node* b) {
    queue_node temp = *a;
    *a = *b;
    *b = temp;
}

// Heapify up to keep min-heap property for insertion
void heapifyUp(PriorityQueue* pq, int index) {
    if (index && pq->queue[(index - 1) / 2].priority > pq->queue[index].priority) { //parent.priority > child.priority
        swap(&pq->queue[(index - 1) / 2], &pq->queue[index]);//swap
        heapifyUp(pq, (index - 1) / 2);//repeat
    }
}

// Add a node to the priority queue
int addNode(PriorityQueue* pq, int x, int y, int priority) {
    //Full Queue
    if (pq->size == MAX) {
        printf("Priority queue is full\n");
        return -1;
    }

    // Insert new node at the end
    pq->queue[pq->size].x_location = x;
    pq->queue[pq->size].y_location = y;
    pq->queue[pq->size].priority = priority;
    
    // Restore heap property
    heapifyUp(pq, pq->size++);
    return 0;
}

// Heapify down to keep min-heap property for deletion
void heapifyDown(PriorityQueue* pq, int index) {
    int smallest = index; //Parent
    int left = 2 * index + 1; //Left Child
    int right = 2 * index + 2; //Right Child

    //Compare priorities (distances)
    if (left < pq->size && pq->queue[left].priority < pq->queue[smallest].priority)
        smallest = left;

    if (right < pq->size && pq->queue[right].priority < pq->queue[smallest].priority)
        smallest = right;

    if (smallest != index) {
        swap(&pq->queue[index], &pq->queue[smallest]);//swap
        heapifyDown(pq, smallest);//Repeat
    }
}

// Dequeue function to remove the node with the lowest priority
int extractNode(PriorityQueue* pq, queue_node* minNode) {
    //Check if empty
    if (!pq->size) {
        printf("Priority queue is empty\n");
        return -1; 
    }

    queue_node min = pq->queue[0]; // Extract the root (min node)
    pq->queue[0] = pq->queue[--pq->size]; // Replace with the last node
    heapifyDown(pq, 0); // Restore heap property
    //Put min into minNode for "return value" 
    minNode->priority = min.priority;
    minNode->x_location = min.x_location;
    minNode->y_location = min.y_location;
    return 0;
}

// Peek function to get the top node without removing it
int peek(PriorityQueue* pq, queue_node* minNode) {
    if (!pq->size) {
        printf("Priority queue is empty\n");
        return -1; // Return an invalid node
    }
    minNode->priority = pq->queue[0].priority;
    minNode->x_location = pq->queue[0].x_location;
    minNode->y_location = pq->queue[0].y_location;
    return 0;
}

// Function to change priority of a node (Dijkstra's algorithm requires decreasing priority)
void decrease_priority(PriorityQueue* pq, int x, int y, int new_priority) {
    // Find the node in the heap
    int index = -1;
    // Search for node
    for (int i = 0; i < pq->size; i++) {
        if (pq->queue[i].x_location == x && pq->queue[i].y_location == y) {
            index = i;
            break;
        }
    }

    // If node not found or priority is already lower, do nothing
    if (index == -1 || pq->queue[index].priority <= new_priority) {
        return;
    }

    // Update priority and restore heap property
    pq->queue[index].priority = new_priority;
    heapifyUp(pq, index);
}
