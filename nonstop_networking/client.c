/**
 * nonstop_networking
 * CS 241 - Fall 2021
 */
#include "format.h"
#include <sys/epoll.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/stat.h>

#include "common.h"

char **parse_args(int argc, char **argv);
verb check_args(char **args);

int connect_to_server(const char *host, const char *port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo aInfo;
    struct addrinfo *res;

    memset(&aInfo, 0, sizeof(struct addrinfo));

    aInfo.ai_family = AF_INET;
    aInfo.ai_socktype = SOCK_STREAM;

    int x = getaddrinfo(host, port, &aInfo, &res);

    if (x != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(x));
        freeaddrinfo(res);
        exit(1);
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("cannot connect");
        freeaddrinfo(res);
        exit(1);
    }

    freeaddrinfo(res);
    return sock;
}

void write_to_server(char** args, verb req, int sock) {
    // 0 = host
    // 1 = port
    // 2 = verb
    // 3 = remote
    // 4 = local
    char * str;

    if (req == LIST) {
        str = calloc(1, strlen(args[2]) + 2);
        sprintf(str, "%s\n", args[2]);
    } else {
        str = calloc(1, strlen(args[2]) + strlen(args[3]) + 3);
        sprintf(str, "%s %s\n", args[2], args[3]);
    }
    ssize_t len = strlen(str);
    ssize_t written = write_all_to_socket(sock, str, len);
    if (written < len) {
        print_connection_closed();
        exit(-1);
    }

    if (req == PUT) {
        struct stat stats;
        int status = stat(args[4], &stats);
        if (status == -1) exit(-1);
        ssize_t size = stats.st_size;
        write_all_to_socket(sock, (char*)&size, sizeof(size_t));
        FILE * readFile = fopen(args[4], "r");
        if(readFile == NULL) exit(-1);
        ssize_t writeSize = 0;
        ssize_t written = 0;
        while (written < size) {
            if ((size - written) > 1024) {
                writeSize = 1024;
            } else {
                writeSize = size - written;
            }
            char buff[writeSize + 1];
            fread(buff, 1, writeSize, readFile);
            ssize_t toCompare = write_all_to_socket(sock, buff, writeSize);
            if (toCompare < writeSize) {
                print_connection_closed();
                exit(-1);
            }
            written += writeSize;
        }
        fclose(readFile);
    }
    free(str);

    int closeStatus = shutdown(sock, SHUT_WR);
    if (closeStatus != 0) perror("error shutting down");
}

void read_from_server(char**args, verb req, int sock) {
    // as specified in the docs
    // 0 = host
    // 1 = port
    // 2 = verb
    // 3 = remote
    // 4 = local

    char * ok = "OK\n";
    char * error = "ERROR\n";
    char * result = calloc(1, sizeof(ok));
    size_t read = read_all_from_socket(sock, result, strlen(ok));
    
    if (strcmp(result, ok) == 0) {
        fprintf(stdout, "%s", result);
        if (req == DELETE || req == PUT) print_success(); 
        else if (req == GET) {
            FILE * f = fopen(args[4], "a+");
            if(f == NULL) exit(-1);
            size_t size;
            read_all_from_socket(sock, (char*)&size, sizeof(size_t));
            size_t n = 0;
            size_t readSize;
            
            while (n < size + 5) {
                if (size - n + 5 > 1024) {
                    readSize = 1024;
                } else {
                    readSize = size - n + 5;
                }
                char buff[1024] = {0};
                size_t readCount = read_all_from_socket(sock, buff, readSize);
                fwrite(buff, 1, readCount, f);
                n += readCount;
                if (readCount == 0) break;
            }

            if (n == 0 && size != 0) {
                print_connection_closed();
                exit(-1);
            } else if (n < size) {
                print_too_little_data();
                exit(-1);
            } else if (n > size) {
                print_received_too_much_data();
                exit(-1);
            }
            fclose(f);
        } else if (req == LIST) {
            size_t size;
            read_all_from_socket(sock, (char*)&size, sizeof(size_t));
            char buff[size + 6];
            memset(buff, 0, size + 6);
            if (read == 0 && size != 0) {
                print_connection_closed();
                exit(-1);
            } else if (read < size) {
                print_too_little_data();
                exit(-1);
            } else if (read > size) {
                print_received_too_much_data();
                exit(-1);
            }
            fprintf(stdout, "%zu%s", size, buff);
        }
    } else {
        result = realloc(result, sizeof(error));
        read_all_from_socket(sock, result + read, strlen(error) - read);
        if (strcmp(result, error) == 0) {
            fprintf(stdout, "%s", result);
            char error_msg[20] = {0};
            if (!read_all_from_socket(sock, error_msg, 20)) print_connection_closed();
            print_error_message(error_msg);
        } else {
            print_invalid_response();
        }
    }
    free(result);
    shutdown(sock, SHUT_RD);
}

int main(int argc, char **argv) {
    // Good luck!

    // 0 = host
    // 1 = port
    // 2 = verb
    // 3 = remote
    // 4 = local
    char ** args = parse_args(argc, argv);
    verb req = check_args(args);

    int sock = connect_to_server(args[0], args[1]);

    

    // put and list
    write_to_server(args, req, sock);
    
    read_from_server(args, req, sock);

    close(sock);

    for (int i = 0; i < argc; i++) {
        free(args[i]);
    }
    free(args);
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}

