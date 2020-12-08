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

void *producer(void *arg) {
    Queue *queue = (Queue *)arg;
	for(int i = 0; ; i++) {
        usleep(1000);
		char buf[40];
		sprintf(buf, "Message %d from thread %d", i, pthread_self());
		queue->mymsgput(buf);
        printf("put\n");
	}
}

void *consumer(void *arg) {
    Queue *queue = (Queue *)arg;
	for (;;) {
        sleep(2);
		char buf[41];
        queue->mymsgget(buf, sizeof(buf));
		printf("Received by thread %d: %s\n", pthread_self(), buf);
        printf("get\n");
	}
}

bool interrupt = false;

void exit_sig(int sig) {
	interrupt = true;
}

int main(int argc, char **argv) {
	sigset(SIGINT, exit_sig);

    Queue *queue = new Queue();

    pthread_t threads[PRODUCERS + CONSUMERS];
    int error;
    for (int i = 0; i < PRODUCERS; i++) {
        error = pthread_create(&threads[i], NULL, producer, queue);
        if (error != SUCCESS) {
            error_exit("pthread_create() failed", error);
        }
    }
	for (int i = 0; i < CONSUMERS; i++) {
        error = pthread_create(&threads[PRODUCERS + i], NULL, consumer, queue);
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

    delete queue;

	return 0;	
}
