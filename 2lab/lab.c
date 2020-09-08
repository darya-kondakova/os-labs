#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define SUCCESS 0

void pthread_error(const char *const msg, int error) {
	fprintf(stderr, "%s: %s", msg, strerror(error));
    exit(1);
}

void print_str(char *str) {
	for (int i = 0; i < 10; i++) {
		printf("%s\n", str);
	}
}

void *child_body(void* arg) {
	print_str("child");

	return NULL;
}

int main() {
	pthread_t child;
	int error = pthread_create(&child, NULL, child_body, NULL);
	if (error != SUCCESS) {
		pthread_error("pthread_create() failed", error);
	}
	
	error = pthread_join(child, NULL);
	if (error != SUCCESS) {
		pthread_error("pthread_join() failed", error);
	}

	print_str("parent");

	pthread_exit(NULL);
}
