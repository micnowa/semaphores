//
// Created by root on 11/22/19.
//

#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


bool push(queue *q, const char *c, int priority) {
    if (!_is_initialized(q)) {
        return false;
    }

    if (!_is_full(q)) {
        int pos = 0;
        for (int j = 0; j < COM_LEN; j++) {
            char *characters = malloc(sizeof(char) * (strlen(q->str[j]) + 2));
            for (int i = 0; i <= (int) strlen(q->str[j]); i++) {
                if (q->priorities[i] >= priority && (q->str[j])[i] != '\0') {
                    characters[i] = (q->str[j])[i];
                } else {
                    pos = i;
                    break;
                }
            }
            characters[pos] = c[j];
            for (int i = pos; i < (int) strlen(q->str[j]); i++) {
                characters[i + 1] = (q->str[j])[i];
            }
            characters[strlen(q->str[j]) + 1] = '\0';
            strcpy(q->str[j], characters);
            free(characters);
        }

        int *integers = malloc(sizeof(int) * q->max);
        for (int i = 0; i < strlen(q->str[0]); i++) {
            if (i == pos) q->priorities[i] = priority;
            else if (i > pos) q->priorities[i] = q->priorities[i + 1];
        }

        free(integers);

        return true;
    }

    return false;
}


char *pop(queue *q) {
    if (_is_empty(q)) return (char *) 0;
    char *c = malloc(sizeof(char) * (COM_LEN + 1));
    for (int j = 0; j < COM_LEN; j++) {
        c[j] = (q->str[j])[0];
        for (int i = 0; i < q->max; i++) (q->str[j])[i] = (q->str[j])[i + 1];
        if (j == 0) for (int i = 0; i < q->max - 1; i++) q->priorities[i] = q->priorities[i + 1];
    }
    c[COM_LEN] = '\0';
    return c;
}


bool destroy_queue(queue *q) {
    free(q->str);
    return true;
}


void print_queue(queue *q) {
    if (!strlen(q->str[0])) puts("Queue empty");
    for (unsigned long i = 0; i < strlen(q->str[0]); i++) {
        printf("%s, %d\n", q->str[i], q->priorities[i]);
    }
}


bool _is_initialized(queue *q) { return q != NULL; }


bool _is_empty(queue *q) {
    return strlen(q->str[0]) == 0;
}


bool _is_full(queue *q) {
    return strlen(q->str[0]) == q->max;
}
