#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <strings.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>

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

int client_body(char *hostname, int port, char *url);

char *copy_bras(const char *string, regmatch_t match[], int brasno) {
    if (match[brasno].rm_so == -1 || match[brasno].rm_so == match[brasno].rm_eo) {
        return NULL;
    }
    int length = match[brasno].rm_eo - match[brasno].rm_so;
    assert(length > 0);

    char *str = malloc(length + 1);
    memcpy(str, string + match[brasno].rm_so, length);
    str[length] = '\0';
    return str;
}

void parse_url(char *url, char **protocol, char **user, char **port, char **hostname, char **uri) {
    const char *url_regexp = "^([a-z]+)://((.*)@)?([a-zA-Z][-a-zA-Z0-9.]*)(:([0-9]+))?(.*)$";
    regex_t compiled_regexp;
    regmatch_t match[10];

    int nb = regcomp(&compiled_regexp, url_regexp, REG_EXTENDED);
    if (nb == FAIL) {
        error_exit("regerrno %s", nb);
    }

    nb = regexec(&compiled_regexp, url, 10, match, 0);
    if (nb != SUCCESS) {
        error_exit("regexp match: %s", nb);
    }

    *protocol = copy_bras(url, match, 1);
    *user = copy_bras(url, match, 3);
    *hostname = copy_bras(url, match, 4);
    *port = copy_bras(url, match, 6);
    *uri = copy_bras(url, match, 7);
}

int open_socket(char *hostname, int port) {
    int res;
    struct hostent *server_host = getipnodebyname(hostname, AF_INET, 0, &res);
    if (server_host == NULL) {
        error_exit("nslookup for %s fail: %s\n", res);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    memcpy(&servaddr.sin_addr.s_addr, server_host->h_addr_list[0], sizeof(struct in_addr));

    freehostent(server_host);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == FAIL) {
        error_exit("socket() failed", ERRNO_SET);
    }
    res = connect(socket_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (res != SUCCESS) {
        error_exit("connect() failed", ERRNO_SET);

    }

    return socket_fd;
}

char *generate_get_command(const char *url, char *buf, size_t size) {
    char *pattern = "GET %.*s HTTP/1.0\r\n\r\n";
    sprintf(buf, pattern, size - strlen(pattern), url);
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 1) {
        error_exit("Usage: ./a.out url", ERRNO_SET);
    }

    char *protocol, *user, *port_string, *hostname, *uri;
    parse_url(argv[1], &protocol, &user, &port_string, &hostname, &uri);

    if (strcmp(protocol, "http")) {
        error_exit("Only HTTP protocol is supported", ERRNO_SET);
    }
    if (user != NULL) {
        error_exit("HTTP authentication is not supported", ERRNO_SET);
    }

    int port = port_string ? atol(port_string) : 80;

    if (port == 0) {
        error_exit("URL format error, port must be numeric", ERRNO_SET);
    }

    client_body(hostname, port, argv[1]);
}
