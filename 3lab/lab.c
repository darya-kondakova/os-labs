#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define SUCCESS 0
#define THREADS 4
#define BUF_SIZE 256

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
        char *child_p[THREADS][BUF_SIZE] = {{"child 1 line 1", "child 1 line 2", "child 1 line 3", NULL},
                {"child 2 line 1", "child 2 line 2", "child 2 line 3", NULL},
                {"child 3 line 1", "child 3 line 2", "child 3 line 3", NULL},
                {"child 4 line 1", "child 4 line 2", "child 4 line 3", NULL}};

        pthread_t child[THREADS];
        int error;
        for (int i = 0; i < THREADS; i++) {
                error = pthread_create(&child[i], NULL, child_body, child_p[i]);
                if (error != 0) {
                        pthread_error("pthread_create(&child[0]) failed", error);
                }
        }

        for (int i = 0; i < THREADS; i++) {
                error = pthread_join(child[i], NULL);
                if (error != SUCCESS) {
                        pthread_error("pthread_join() failed", error);
                }
        }

        pthread_exit(NULL);
}
