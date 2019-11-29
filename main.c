#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <wait.h>
#include <errno.h>
#include <time.h>
#include "shared_memory.h"
#include "queue.h"

#define QUEUE_SIZE 50
#define QUEUE_NUM 3
#define FOR_QUEUES for(int i=0; i<QUEUE_NUM; i++)


/** Semaphores **/
char *sem_mutex_name[QUEUE_NUM] = {"sem_mutex_0", "sem_mutex_1", "sem_mutex_2"};
char *sem_empty_name[QUEUE_NUM] = {"sem_empty_0", "sem_empty_1", "sem_empty_2"};
char *sem_full_name[QUEUE_NUM] = {"sem_full_0", "sem_full_1", "sem_full_2"};

sem_t *sem_mutex[QUEUE_NUM];
sem_t *sem_empty[QUEUE_NUM];
sem_t *sem_full[QUEUE_NUM];

int QUEUE_KEY = 0;
int qt_id;
int shmid[QUEUE_NUM];

typedef enum {
    LOW, HIGH
} priority;

char com[] = {'A', 'B', 'C'};
int pri[] = {LOW, HIGH};
double pr;

void free_shared_memory(queue **q) {
    shmdt(q);
    shmctl(qt_id, IPC_RMID, 0);
}

int draw_number() {
    return rand() % QUEUE_NUM;
}

void produce(queue **q, char communicate, int num, int priority) {
    sem_wait(sem_empty[num]);
    sem_wait(sem_mutex[num]);
    push(q[num], communicate, priority);
    sem_post(sem_mutex[num]);
    sem_post(sem_full[num]);
    printf("Added %c, with priority: %d\n", communicate, priority);
}

void consume(queue **q, int num) {
    sem_wait(sem_full[num]);
    sem_wait(sem_mutex[num]);
    char tmp = pop(q[num]);
    sem_post(sem_mutex[num]);
    sem_post(sem_empty[num]);
    printf("Consumed: %c\n", tmp);
}

void initialize_semaphores() {
    for (int i = 0; i < QUEUE_NUM; i++) {
        sem_mutex[i] = sem_open(sem_mutex_name[i], O_CREAT | O_EXCL, 0644, 1);
        sem_empty[i] = sem_open(sem_empty_name[i], O_CREAT | O_EXCL, 0644, QUEUE_SIZE);
        sem_full[i] = sem_open(sem_full_name[i], O_CREAT | O_EXCL, 0644, 0);
    }
}

void destroy_semaphores() {
    for (int i = 0; i < QUEUE_NUM; i++) {
        sem_unlink(sem_full_name[i]);
        sem_unlink(sem_mutex_name[i]);
        sem_unlink(sem_empty_name[i]);
    }
}


int main(void) {
    int i;
    sem_t *sem;
    pid_t pid;
    unsigned int n = 7;
    unsigned int value = 1;
    int freq = 1;
    float pr = 0.2;

    srand(time(0));

    /* initialize semaphores for shared processes */
    sem = sem_open("pSem", O_CREAT | O_EXCL, 0644, value);
    initialize_semaphores();

    /* name of semaphore is "pSem", semaphore is reached using this name */

    printf("semaphores initialized.\n\n");

    queue **q;
    qt_id = shmget(QUEUE_KEY, 3 * sizeof(queue *), IPC_CREAT | 0666);
    q = (queue **) shmat(qt_id, 0, 0);
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


    /* fork child processes with checking if it's possible*/
    for (i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            sem_unlink("pSem");
            sem_close(sem);
            printf("Fork error.\n");
        } else if (pid == 0) break;
    }


    /******************   PARENT PROCESS   ****************/
    if (pid != 0) {
        /* wait for all children to exit */
        while (pid = waitpid(-1, NULL, 0)) if (errno == ECHILD) break;
        printf("\nParent: All children have exited.\n");
        queue *queue1 = q[0];
        free_shared_memory(q);
        sem_unlink("pSem");
        sem_close(sem);
        destroy_semaphores();
        exit(0);
    }

        /******************   CHILD PROCESS   *****************/
    else {
        sem_wait(sem);
        if (i == 0 || i == 1 || i == 2) printf("Producer(%d) started running\n", i);
        else if (i == 3 || i == 4 || i == 5) printf("Consumer(%d) started running\n", i%3);
        else printf("Special producer started running\n");
        sleep(1);
        sem_post(sem);


        char signs[3];
        int added = 3;
        if (i == 0 || i == 1 || i == 2) { /** Producer **/
            for (int j = 0; j < added; j++) signs[j] = com[draw_number()];
            for (int j = 0; j < added; j++) {
                printf("%d\n", draw_number());
                sem_wait(sem_empty[i]);
                sem_wait(sem_mutex[i]);
                push(q[i], signs[j], 0);
                sem_wait(sem);
                printf("Added %c to %d\n", signs[j], i);
                sem_post(sem);
                sem_post(sem_mutex[i]);
                sem_post(sem_full[i]);
            }

        } else if (i == 3 || i == 4 || i == 5) { /** Consumer  **/
            for (int j = 0; j < added; j++) {
                sem_wait(sem_full[i % 3]);
                sem_wait(sem_mutex[i % 3]);
                char tmp = pop(q[i%3]);
                sem_wait(sem);
                printf("Consumed %c from %d\n", tmp, i%3);
                sem_post(sem);
                sem_post(sem_mutex[i % 3]);
                sem_post(sem_empty[i % 3]);
            }
        } else if (i == 6) { /** Special producer **/
            int queue_number = draw_number();
            //for (int j = 0; j < added; j++) signs[i] = com[draw_number()];
        }

    }

    exit(0);
}


