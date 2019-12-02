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
#include <string.h>
#include "shared_memory.h"
#include "queue.h"

#define QUEUE_SIZE 10
#define QUEUE_NUM 3
#define MS_TO_US 1000

/** Semaphores **/
char *sem_mutex_name[QUEUE_NUM] = {"sem_mutex_0", "sem_mutex_1", "sem_mutex_2"};
char *sem_empty_name[QUEUE_NUM] = {"sem_empty_0", "sem_empty_1", "sem_empty_2"};
char *sem_full_name[QUEUE_NUM] = {"sem_full_0", "sem_full_1", "sem_full_2"};

sem_t *sem_mutex[QUEUE_NUM];
sem_t *sem_empty[QUEUE_NUM];
sem_t *sem_full[QUEUE_NUM];

int QUEUE_KEY = 0;
int qq_mem_id;
int q_mem_id[QUEUE_NUM];
int q_mem_pri_id[QUEUE_NUM];
int q_mem_str_id[QUEUE_NUM][COM_LEN];

typedef enum {
    LOW, HIGH
} priority;

char com[] = {'A', 'B', 'C'};
double pr = 0.2;

void produce(queue **q, char *communicate, int num, int priority, sem_t *empty, sem_t *mutex, sem_t *full) {
    sem_wait(empty);
    sem_wait(mutex);
    push(q[num], communicate, priority);
    sem_post(mutex);
    sem_post(full);
}

char *consume(queue **q, int num, sem_t *empty, sem_t *mutex, sem_t *full) {
    sem_wait(full);
    sem_wait(mutex);
    char *tmp = pop(q[num]);
    sem_post(mutex);
    sem_post(empty);
    return tmp;
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

void handle_signal(int SIG) {
    puts("Signal caught!");
}

char *int_to_str(int num) {
    char *str = malloc(sizeof(char) * 20);
    sprintf(str, "%d", num);
    return str;
}

char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void destroy_shmem(int shmid) {
    char *command = "ipcrm -m ";
    char *str = int_to_str(shmid);
    char *final_command = concat(command, str);
    system(final_command);
}

void remove_queue_from_shmem() {
    /** 3 strings for each queue **/
    for (int i = 0; i < QUEUE_NUM; i++) {
        for (int j = 0; j < COM_LEN; j++) {
            destroy_shmem(q_mem_str_id[i][j]);
        }
    }
    /** priorities for each queue **/
    for (int i = 0; i < QUEUE_NUM; i++) {
        destroy_shmem(q_mem_pri_id[i]);
        /** pointers to queues **/
        destroy_shmem(q_mem_id[i]);
    }
    /** pointer to pointers to queues **/
    destroy_shmem(qq_mem_id);
}


int main(void) {
    int times = 20;
    int i;
    sem_t *sem;
    pid_t pid;
    unsigned int n = 7;
    unsigned int value = 1;
    int sleep_ms_time = 500;
    int freq = 500;
    srand(time(NULL));

    /* initialize semaphores for shared processes */
    sem = sem_open("pSem", O_CREAT | O_EXCL, 0644, value);
    initialize_semaphores();
    /* name of semaphore is "pSem", semaphore is reached using this name */

    printf("semaphores initialized.\n\n");

    queue **q;
    qq_mem_id = shmget(QUEUE_KEY, 3 * sizeof(queue *), IPC_CREAT | 0666);
    q = (queue **) shmat(qq_mem_id, 0, 0);
    for (int j = 0; j < QUEUE_NUM; j++) {
        q_mem_id[j] = shmget(QUEUE_KEY, sizeof(queue), IPC_CREAT | 0666);
        q[j] = (queue *) shmat(q_mem_id[j], 0, 0);
        q[j]->max = QUEUE_SIZE;
        q_mem_pri_id[j] = shmget(QUEUE_KEY, (QUEUE_SIZE) * sizeof(int), IPC_CREAT | 0666);
        q[j]->priorities = (int *) shmat(q_mem_pri_id[j], 0, 0);
        for (int k = 0; k < COM_LEN; k++) {
            q_mem_str_id[j][k] = shmget(QUEUE_KEY, (QUEUE_SIZE + 1) * sizeof(char), IPC_CREAT | 0666);
            q[j]->str[k] = (char *) shmat(q_mem_str_id[j][k], 0, 0);
        }
    }


    // fork child processes with checking if it's possible
    for (i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            sem_unlink("pSem");
            sem_close(sem);
            printf("Fork error.\n");
        } else if (pid == 0) break;
    }


    if (pid != 0) {
        signal(SIGINT, handle_signal);
        while (pid = waitpid(-1, NULL, 0)) if (errno == ECHILD) break;
        printf("\nParent: All children have exited.\n");
        sem_unlink("pSem");
        sem_close(sem);
        destroy_semaphores();
        remove_queue_from_shmem();
        exit(0);
    } else {
        sem_wait(sem);
        if (i == 0 || i == 1 || i == 2) printf("Producer(%d) started running\n", i);
        else if (i == 3 || i == 4 || i == 5) printf("Consumer(%d) started running\n", i % 3);
        else if(i== 6) printf("Special producer(%d) started running\n", i%6);
        sem_post(sem);
        sleep(1);

        if (i == 0 || i == 1 || i == 2) { // Producer
            printf("Producer %d\n", i);
            int counter = 0;
            char *signs = malloc(sizeof(char) * (COM_LEN + 1));
            signs[COM_LEN] = '\0';
            while (counter != times) {
                for (int j = 0; j < COM_LEN; j++) signs[j] = com[rand() % COM_LEN];
                usleep(MS_TO_US * sleep_ms_time);
                produce(q, signs, i, LOW, sem_empty[i], sem_mutex[i], sem_full[i]);
                sem_wait(sem);
                printf("Added %s(0) to q[%d]\n", signs, i);
                sem_post(sem);
                usleep(MS_TO_US * freq);
                counter++;
            }


        } else if (i == 3 || i == 4 || i == 5) {// Consumer
            printf("Consumer %d\n", i);
            int k = i % 3;
            int counter = 0;
            while (counter != times) {
                char *tmp;
                tmp = consume(q, k, sem_empty[k], sem_mutex[k], sem_full[k]);
                sem_wait(sem);
                printf("Consumed %s from q[%d]\n", tmp, k);
                sem_post(sem);

                usleep(MS_TO_US * freq);
                double r = ((double) rand() / (RAND_MAX));
                if (r < pr) {
                    char *signs = malloc(sizeof(char) * (COM_LEN + 1));
                    signs[COM_LEN] = '\0';
                    for (int m = 0; m < COM_LEN - 1; m++) {
                        signs[m] = signs[m + 1];
                    }
                    signs[0] = com[rand() % COM_LEN];
                    produce(q, signs, i, LOW, sem_empty[i], sem_mutex[i], sem_full[i]);
                }
                counter++;
            }
        } else if(i == 6) { // Special producer
            printf("Special producer: %d\n", i);
            int counter = 0;
            char *signs = malloc(sizeof(char) * (COM_LEN + 1));
            signs[COM_LEN] = '\0';
            while (counter != times) {
                int queue_number = rand() % QUEUE_NUM;
                for (int j = 0; j < COM_LEN; j++) signs[j] = com[rand() % COM_LEN];
                produce(q, signs, queue_number, HIGH, sem_empty[queue_number], sem_mutex[queue_number], sem_full[queue_number]);
                sem_wait(sem);
                printf("Drawn queue: %d\n", queue_number);
                printf("Added %s(1) to q[%d]\n", signs,queue_number);
                sem_post(sem);
                usleep(MS_TO_US * sleep_ms_time);
                counter++;
            }
        }

    }
    exit(0);
}


