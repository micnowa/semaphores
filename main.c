#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared_memory.h"
#include "queue.h"

//#define KEY "key"

#define QUEUE_SIZE 20
#define QUEUE_NUM 3

/** Semaphores **/
char *sem_mutex_name[QUEUE_SIZE] = {"sem_mutex_0", "sem_mutex_1", "sem_mutex_2"};
char *sem_empty_name[QUEUE_SIZE] = {"sem_empty_0", "sem_empty_1", "sem_empty_2"};
char *sem_full_name[QUEUE_SIZE] = {"sem_full_0", "sem_full_1", "sem_full_2"};
int QUEUE_KEY = 0;

typedef enum {
    LOW, HIGH
} priority;


char com[] = {'A', 'B', 'C'};
int pri[] = {LOW, HIGH};
double pr;

int draw_number() {
    return rand() % QUEUE_NUM;
}

void produce(queue **q, int priority) {
    int n = 200, i = 0;
    while (n != (i++)) {
        int pos = draw_number();
        char communicate = com[pos];
        sem_t *full = sem_open(sem_full_name[pos], 0);
        sem_t *mutex = sem_open(sem_mutex_name[pos], 0);
        sem_t *empty = sem_open(sem_empty_name[pos], 0);
        sem_wait(empty);
        sem_wait(mutex);
        priority = i % 2;
        push(q[pos], communicate, priority);
        sem_post(mutex);
        sem_post(full);
    }
}

void initialize_semaphores() {
    for (int i = 0; i < QUEUE_NUM; i++) {
        sem_open(sem_mutex_name[i], O_CREAT, 0644, 1);
        sem_open(sem_empty_name[i], O_CREAT, 0644, QUEUE_SIZE);
        sem_open(sem_full_name[i], O_CREAT, 0644, 0);
    }
}

void destroy_semaphores() {
    for (int i = 0; i < QUEUE_NUM; i++) {
        sem_destroy(sem_open(sem_full_name[i], 0));
        sem_destroy(sem_open(sem_mutex_name[i], 0));
        sem_destroy(sem_open(sem_empty_name[i], 0));
    }
}

bool producer(queue *q[]) {
    puts("Producer");
    initialize_semaphores();
    produce(q, LOW);
    return true;
}

bool special_producer(queue *q[]) {
    puts("Special producer");
    produce(q, HIGH);
    return true;
}


bool consumer(queue *q[]) {
    puts("Consumer");
    sleep(3);

    for (int i = 0; i < QUEUE_NUM; i++) {
        print_queue(q[i]);
        char tmp = '0';
        sem_t *full = sem_open(sem_full_name[i], 0);
        sem_t *mutex = sem_open(sem_mutex_name[i], 0);
        sem_t *empty = sem_open(sem_empty_name[i], 0);
        while (tmp != (char) 0) {
            sem_wait(full);
            sem_wait(mutex);
            tmp = pop(q[i]);
            sem_post(mutex);
            sem_post(empty);
        }
        print_queue(q[i]);
        puts("-----");
    }
    return true;
}


int main() {
    queue **queue_prototype = malloc(sizeof(queue *));
    for (int i = 0; i < QUEUE_NUM; i++) queue_prototype[i] = create_queue(QUEUE_SIZE);

    queue **q;


    int qt_id = shmget(QUEUE_KEY, 3 * sizeof(queue *), IPC_CREAT | 0666);
    q = (queue **) shmat(qt_id, 0, 0);
    int shmid[QUEUE_NUM];
    for (int i = 0; i < QUEUE_NUM; i++) {
        shmid[i] = shmget(QUEUE_KEY, sizeof(queue), IPC_CREAT | 0666);
        q[i] = (queue *) shmat(shmid[i], 0, 0);
        q[i]->max = QUEUE_SIZE;
    }
    for (int i = 0; i < QUEUE_NUM; i++) {
        shmid[i] = shmget(QUEUE_KEY, (QUEUE_SIZE) * sizeof(int), IPC_CREAT | 0666);
        q[i]->priorities = (int *) shmat(shmid[i], 0, 0);
        shmid[i] = shmget(QUEUE_KEY, (QUEUE_SIZE + 1) * sizeof(char), IPC_CREAT | 0666);
        q[i]->str = (char *) shmat(shmid[i], 0, 0);
    }

    if (fork()) {
        if (fork()) {
            producer(q);
        } else {
            consumer(q);
        }
    } else {
        special_producer(q);
    }

    return 0;
}