#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define SUCCESS 0

void pthread_error(const char *const msg, int error) {
	fprintf(stderr, "%s: %s", msg, strerror(error));
    exit(1);
}

void *child_body(void* arg) {
	for (;;) {
		write(0, "child\n", 6);
	}

	return NULL;
}

int main() {
	pthread_t child;
	int error = pthread_create(&child, NULL, child_body, NULL);
	if (error != 0) {
		pthread_error("pthread_create() failed", error);
	}
	
	sleep(2);

	error = pthread_cancel(child);
	if (error != 0) {
		pthread_error("pthread_cancel() failed", error);
	}	

	pthread_exit(NULL);
}
