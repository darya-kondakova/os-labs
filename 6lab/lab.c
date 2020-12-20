#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define FILE_BUFFER_SIZE 256
#define SUCCESS 0
#define FAIL -1
#define ERRNO_SET 0
#define ERROR_EXIT 1

void error_exit(const char *const msg, const int error) {
    if (error != ERRNO_SET) {
        fprintf(stderr, "%s: %s", msg, strerror(error));
    } else {
        if (errno == 0) {
            fprintf(stderr, "%s\n", msg);
        } else {
            perror(msg);
        }
    }
    exit(ERROR_EXIT);
}

void free_charsets(char **charsets) {
    free(charsets[0]);
    free(charsets[1]);
    free(charsets);
}

char **build_new_paths(const char *sourcePath, const char *destinationPath, const char *additionalPath) {
    size_t additionalLen = strlen(additionalPath),
            sourcePathLen = strlen(sourcePath) + 1 + additionalLen,
            destinationPathLen = strlen(destinationPath) + 1 + additionalLen;

    char **new_paths = (char **) malloc(sizeof(char *) * 2);
    new_paths[0] = (char *) malloc(sizeof(char) * sourcePathLen);
    new_paths[1] = (char *) malloc(sizeof(char) * destinationPathLen);

    strcpy(new_paths[0], sourcePath);
    strcat(new_paths[0], "/");
    strcat(new_paths[0], additionalPath);

    strcpy(new_paths[1], destinationPath);
    strcat(new_paths[1], "/");
    strcat(new_paths[1], additionalPath);

    return new_paths;
}

void *cpFunction(void *arg);

size_t direntLen;

void copyFolder(const char *sourcePath, const char *destinationPath, mode_t mode) {
    DIR *dir;
    for(;;) {
        dir = opendir(sourcePath);
        if (dir == NULL) {
            if (errno != EMFILE) {
                fprintf(stderr, "Couldn't open directory %s, %s\n", sourcePath, strerror(errno));
                return;
            }
            sleep(1);
            continue;
        }
        break;
    }

    int ret_val = mkdir(destinationPath, mode);
    if (ret_val != SUCCESS) {
        fprintf(stderr, "Couldn't create directory %s, %s\n", destinationPath, strerror(errno));
        return;
    }

    struct dirent *entry = (struct dirent *) malloc(direntLen);
    if (entry == NULL) {
        fprintf(stderr, "malloc() failed, %s", strerror(errno));
        return;
    }
    
    struct dirent *result;
    for(;;) {
        ret_val = readdir_r(dir, entry, &result);
        if (ret_val != SUCCESS || result == NULL) {
            break;
        }
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char **newPaths = build_new_paths(sourcePath, destinationPath, entry->d_name);

        pthread_t thread;
        for (;;) {
            ret_val = pthread_create(&thread, NULL, cpFunction, (void *) newPaths);
            if (ret_val != SUCCESS) {
                if (ret_val != EAGAIN) {
                    free_charsets(newPaths);
                    fprintf(stderr, "Couldn't copy %s, %s\n", newPaths[0], strerror(errno));
                    break;
                }
                sleep(1);
                continue;
            }
            break;
        }
    }
    free(entry);
    ret_val = closedir(dir);
    if (ret_val != SUCCESS) {
        perror("closedir() failed");
    }
}

void copyFile(const char *sourcePath, const char *destinationPath, mode_t mode) {
    int fdin, fdout;
    for (;;) {
        fdin = open(sourcePath, O_RDONLY);
        if (fdin == FAIL) {
            if (errno != EMFILE) {
                fprintf(stderr, "Couldn't open file %s, %s\n", sourcePath, strerror(errno));
                return;
            }
            sleep(1);
            continue;
        }
        break;
    }
    for (;;) {
        fdout = open(destinationPath, O_WRONLY | O_CREAT | O_EXCL, mode);
        if (fduot == FAIL) {
            if (errno != EMFILE) {
                fprintf(stderr, "Couldn't open file %s, %s\n", destinationPath, strerror(errno));
                if (close(fdin) != SUCCESS) {
                    perror("close() failed");
                }
                return;
            }
            sleep(1);
            continue;
        }
        break;
    }
    
    for (;;) {
        int bytes_read = read(fdin, buffer, FILE_BUFFER_SIZE);
        if (bytesRead > 0) {
            int bytes_write write(fdout, buffer, bytesRead);
            if (bytes_write == FAIL) {
                perror("write() failed");
            }
            continue;
        }
        if (bytesRead == FAIL) {
            perror("read() failed");
        }
        break;
    }

    int ret_val = close(fdin);
    if (ret_val != SUCCESS) {
        perror("close() failed");
    }
    ret_val = close(fdout);
    if (ret_val != SUCCESS) {
        perror("close() failed");
    }
}

void *cpFunction(void *arg) {
    struct stat statBuffer;
    char *sourcePath = ((char **) arg)[0];
    char *destinationPath = ((char **) arg)[1];
    
    int error = stat(sourcePath, &statBuffer);
    if (error != SUCCESS) {
        error_exit("stat() failed\n", ERRNO_SET);
    }
    if (S_ISDIR(statBuffer.st_mode)) {
        copyFolder(sourcePath, destinationPath, statBuffer.st_mode);
    }
    if (S_ISREG(statBuffer.st_mode)) {
        copyFile(sourcePath, destinationPath, statBuffer.st_mode);
    }
    free_charsets(arg);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        error_exit("Usage: ./a.out <copy source> <copy destination>\n", 0);
    }

    char *copy[2];
    for (int i = 0; i < 2; i++) {
        copy[i] = (char *)malloc(sizeof(char) * strlen(argv[i + 1]));
        strcpy(copy[i], argv[i + 1]);
        if (copy[i][strlen(argv[i + 1]) - 1] == '/') {
            copy[i][strlen(argv[i + 1]) - 1] = '\0';
        }
    }
    size_t pathlen = pathconf(argv[1], _PC_NAME_MAX);
    pathlen = (pathlen == -1) ? 255 : pathlen;
    direntLen = offsetof(struct dirent, d_name) + pathlen + 1;

    cpFunction(copy);

    pthread_exit(NULL);
}
