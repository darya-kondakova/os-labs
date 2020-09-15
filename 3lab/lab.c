#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define SUCCESS 0

void pthread_error(const char *const msg, int error) {
	fprintf(stderr, "%s: %s", msg, strerror(error));
    exit(1);
}

void *child_body(void* arg) {
	for (char **line = arg; *line != NULL; line++) {
		printf("%s\n", *line);
	}

	return NULL;
}

int main() {
	char *child_1[] = {"child 1 line 1", "child 1 line 2", "child 1 line 3", NULL};
	char *child_2[] = {"child 2 line 1", "child 2 line 2", "child 2 line 3", NULL};
	char *child_3[] = {"child 3 line 1", "child 3 line 2", "child 3 line 3", NULL};
	char *child_4[] = {"child 4 line 1", "child 4 line 2", "child 4 line 3", NULL};

	pthread_t child[4];
	int error = pthread_create(&child[0], NULL, child_body, child_1);
	if (error != 0) {
		pthread_error("pthread_create(&child[0]) failed", error);
	}
	error = pthread_create(&child[1], NULL, child_body, child_2);
	if (error != 0) {
		pthread_error("pthread_create(&child[1]) failed", error);
	}
	error = pthread_create(&child[2], NULL, child_body, child_3);
	if (error != 0) {
		pthread_error("pthread_create(&child[2]) failed", error);
	}
	error = pthread_create(&child[3], NULL, child_body, child_4);
	if (error != 0) {
		pthread_error("pthread_create(&child[3]) failed", error);
	}
	
	pthread_exit(NULL);
}
