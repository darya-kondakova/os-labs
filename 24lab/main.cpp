#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <alloca.h>
#include <unistd.h>
#include <string.h>
#include "queue.h"
#include "error_exit.h"

#define PRODUCERS 2
#define CONSUMERS 2
#define SUCCESS 0

typedef struct thread_struct {
	int thread_num;
	Queue *queue;
} thread_t;

void *producer(void *arg) {
    Queue *queue = ((thread_t *)arg)->queue;
	for(int i = 0; ; i++) {
        usleep(1000);
		char buf[40];
		sprintf(buf, "Message %d from thread %d", i, ((thread_t *)arg)->thread_num);
		queue->mymsgput(buf);
	}
}

void *consumer(void *arg) {
    Queue *queue = ((thread_t *)arg)->queue;
	for (;;) {
        sleep(2);
		char buf[41];
        queue->mymsgget(buf, sizeof(buf));
		printf("Received by thread %d: %s\n", ((thread_t *)arg)->thread_num, buf);
	}
}

bool interrupt = false;

void exit_sig(int sig) {
	interrupt = true;
}

int main(int argc, char **argv) {
	sigset(SIGINT, exit_sig);

    thread_t *thread_struct = (thread_t *)malloc((PRODUCERS + CONSUMERS) * sizeof(thread_t));
    if (thread_struct == NULL) {
        error_exit("failed to allocate memory for thread_struct", ERRNO_SET);
    }

    Queue *queue = new Queue();

    pthread_t threads[PRODUCERS + CONSUMERS];
    int error;
    for (int i = 0; i < PRODUCERS; i++) {
        thread_struct[i].thread_num = i;
        thread_struct[i].queue = queue;
        error = pthread_create(&threads[i], NULL, producer, &thread_struct[i]);
        if (error != SUCCESS) {
            error_exit("pthread_create() failed", error);
        }
    }
	for (int i = PRODUCERS; i < PRODUCERS + CONSUMERS; i++) {
        thread_struct[i].thread_num = i;
        thread_struct[i].queue = queue;
        error = pthread_create(&threads[i], NULL, consumer, &thread_struct[i]);
        if (error != SUCCESS) {
            error_exit("pthread_create() failed", error);
        }
	}

    while (!interrupt) {}

	queue->mymsgdrop();

    for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
        error = pthread_cancel(threads[i]);
        if (error != SUCCESS) {
            error_exit("pthread_cancel() failed", error);
        }
    }

	for (int i = 0; i < PRODUCERS + CONSUMERS; i++) {
        error = pthread_join(threads[i], NULL);
        if (error != SUCCESS) {
            error_exit("pthread_join() failed", error);
        }
	}

    free(thread_struct);
    delete queue;

	return 0;	
}
