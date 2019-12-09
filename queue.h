//
// Created by Michał Nowaliński on 11/22/19.
//

#include <stdbool.h>
#include <semaphore.h>

#ifndef LAB3_QUEUE_H
#define LAB3_QUEUE_H

#define PROCESS_SHARE 1
#define COM_LEN 3

typedef struct {
    int max;
    char *str[COM_LEN];
    int *priorities;
} queue;

bool push(queue *q, const char *c, int priority);

char* pop(queue *q);

queue *create_queue(int size);

bool destroy_queue(queue *q);

void print_queue(queue *q);

bool _is_initialized(queue *q);

bool _is_empty(queue *q);

bool _is_full(queue *q);

void flush_queue(queue *q);

#endif //LAB3_QUEUE_H
