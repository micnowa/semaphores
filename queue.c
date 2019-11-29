//
// Created by root on 11/22/19.
//

#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


bool push(queue *q, char c, int priority) {
    if (!_is_initialized(q)) {
        return false;
    }
    if (!_is_full(q)) {
        char *characters = malloc(sizeof(char) * (strlen(q->str) + 2));
        int *integers = malloc(sizeof(int) * q->max);
        int pos = 0;
        for (int i = 0; i <= (int) strlen(q->str); i++) {
            if (q->priorities[i] >= priority && q->str[i] != '\0') {
                characters[i] = q->str[i];
                integers[i] = q->priorities[i];
            } else {
                pos = i;
                break;
            }
        }
        integers[pos] = priority;
        characters[pos] = c;
        for (int i = pos; i < (int) strlen(q->str); i++) {
            characters[i + 1] = q->str[i];
            integers[i + 1] = q->priorities[i];
        }
        characters[strlen(q->str) + 1] = '\0';
        strcpy(q->str, characters);
        for (int i = 0; i <= q->max; i++)q->priorities[i] = integers[i];
        free(characters);
        free(integers);

        return true;
    }

    return false;
}


char pop(queue *q) {
    if (_is_empty(q)) return (char) 0;
    char c = q->str[0];
    int pri = q->priorities[0];
    for (int i = 0; i < q->max; i++) q->str[i] = q->str[i + 1];
    for (int i = 0; i < q->max - 1; i++) q->priorities[i] = q->priorities[i + 1];
    return c;
}


queue *create_queue(int size) {
    queue *q = malloc(sizeof(queue));
    q->str = malloc((size + 1) * sizeof(char));
    q->max = size;
    q->priorities = malloc(sizeof(int) * size);
    return q;
}


bool destroy_queue(queue *q) {
    free(q->str);
    return true;
}


void print_queue(queue *q) {
    if(strlen(q->str)) puts("Queue empty");
    for (unsigned long i = 0; i < strlen(q->str); i++)
        printf("%c, %d\n", q->str[i], q->priorities[i]);
}


bool _is_initialized(queue *q) { return q != NULL; }


bool _is_empty(queue *q) {
    return strlen(q->str) == 0;
}


bool _is_full(queue *q) {
    return strlen(q->str) == q->max;
}


void flush_queue(queue *q) {
    free(q->str);
    free(q->priorities);
    q->str = malloc(sizeof(char) * (q->max + 1));
    q->str = "";
    q->priorities = malloc(sizeof(int) * q->max);
}