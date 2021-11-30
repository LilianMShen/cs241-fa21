/**
 * nonstop_networking
 * CS 241 - Fall 2021
 */
#include "common.h"
#include "format.h"
#include "includes/dictionary.h"
#include "includes/vector.h"

#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

struct client_info {
    verb command;
    int status;
    char file[255];
    char desc[1024];
    size_t t;
};
static dictionary * clients;
static char * directory;
static vector * files;
static dictionary * files_sizes;

static int efd;

// turns everything off
// frees static vars
void off() {
    close (efd);
    dictionary_clear(clients);
    dictionary_clear(files_sizes);
    vector_destroy(files);
    remove(directory);
    exit(1);
}

void in_to_out(int cfd) {
    struct epoll_event event;
    event.events = EPOLLOUT;
    event.data.fd = cfd;
    epoll_ctl(efd, EPOLL_CTL_MOD, cfd, &event);
}

void exec_command(int cfd, struct client_info * cinfo) {
    if (cinfo->command == GET) {
		int len = strlen(directory) + strlen(cinfo->file) + 1;
		char path[len];
		memset(path, 0, len);
		sprintf(path, "%s/%s", directory, cinfo->file);
		FILE *r_mode = fopen(path, "r");
		if (!r_mode) {
			cinfo->status = -3;
			return;
		}
		write_all_to_socket(cfd, "OK\n", 3);
		size_t size;
		size = *(size_t *) dictionary_get(files_sizes, cinfo->file);
		write_all_to_socket(cfd, (char*)&size, sizeof(size_t));
		size_t w_count = 0;
    	while (w_count < size) {
      		size_t size_head;
      		if( (size-w_count) > 1024){
        		size_head = 1024;
      		}else{
        		size_head =  (size - w_count);
      		}
      		char buffer[size_head + 1];
      		fread(buffer, 1, size_head, r_mode);
      		write_all_to_socket(cfd, buffer, size_head);
      		w_count += size_head;
    	}
		fclose(r_mode);
	} else if (cinfo->command == PUT) {
		write_all_to_socket(cfd, "OK\n", 3);
	} else if (cinfo->command == DELETE) {
		int len = strlen(directory) + strlen(cinfo->file) + 2;
		char path[len];
		memset(path, 0, len);
		sprintf(path, "%s/%s", directory, cinfo->file);
		if (remove(path) < 0) {
			cinfo->status = -3;
			return;
		}
		size_t i = 0;
		VECTOR_FOR_EACH(files, name, {
	        if (!strcmp((char *) name, cinfo->file)) break;
			i++;
	 	});
		size_t v_size= vector_size(files);
		if (i == v_size) {
			cinfo->status = -3;
			return;
		}
		vector_erase(files, i);
		dictionary_remove(files_sizes, cinfo->file);
		write_all_to_socket(cfd, "OK\n", 3);
	} else if (cinfo->command == LIST) {
		write_all_to_socket(cfd, "OK\n", 3);
		size_t size = 0;
		VECTOR_FOR_EACH(files, name, {
	         size += strlen(name)+1;
	    });
		if (size) size--;
		write_all_to_socket(cfd, (char*)& size, sizeof(size_t));
		VECTOR_FOR_EACH(files, name, {
		        write_all_to_socket(cfd, name, strlen(name));
				if (_it != _iend-1) write_all_to_socket(cfd, "\n", 1);
		});
	}
	epoll_ctl(efd, EPOLL_CTL_DEL, cfd, NULL);
  	free(dictionary_get(clients, &cfd));
	dictionary_remove(clients, &cfd);
  	shutdown(cfd, SHUT_RDWR);
	close(cfd);
}

int put(int cfd, struct client_info * cinfo) {
    int finalLen = strlen(directory) + strlen(cinfo->file) + 2;
    char * path = calloc(1, finalLen);
    sprintf(path, "%s/%s", directory, cinfo->file);
    FILE *rfile = fopen(path, "r");
    FILE *wfile = fopen(path, "w");

    if (!wfile) {
        perror("fopen");
        return 1;
    }

    size_t size;
    read_all_from_socket(cfd, (char*) &size, sizeof(size_t));
    size_t rcount = 0;
    while (rcount < size + 5) {
        size_t size_head;
        if ((size + 5 - rcount) > 1024) size_head = 1024;
        else size_head = (size + 5 - rcount);

        char buff[1025] = {0};
        ssize_t count = read_all_from_socket(cfd, buff, size_head);
        fwrite(buff, 1, count, wfile);
        rcount += count;
        if (count == 0) break;
    }

    fclose(wfile);
    if (rcount == 0 && rcount != size) {
        print_connection_closed();
        remove(path);
        return 1;
    } 
    if (rcount < size) {
        print_too_little_data();
        remove(path);
        return 1;
    }
    if (rcount > size) {
        print_received_too_much_data();
        remove(path);
        return 1;
    }

    if (!rfile) vector_push_back(files, cinfo->file);
    else fclose(rfile);
    
    dictionary_set(files_sizes, cinfo->file, &size);

    return 0;
}

void get_command(int cfd, struct client_info * cinfo) {
    size_t count = 0;
    // upperbound
    while (count < 1024) {
        ssize_t r = read(cfd, cinfo->desc + count, 1);
        if (r == 0) break;
        if (cinfo->desc[strlen(cinfo->desc) - 1] == '\n') break;
        if (r == -1) {
            if (errno == EINTR) continue;
            else perror("could not read");
        }
        count += r;
    }
    
    if (count == 1024) {
        cinfo->status = -1;
        in_to_out(cfd);
        return;
    }

    if (!strncmp(cinfo->desc, "LIST", 4)) {
        cinfo->command = LIST;
    } else if (!strncmp(cinfo->desc, "PUT", 3)) {
        cinfo->command = PUT;
        strcpy(cinfo->file, cinfo->desc + 4);
        cinfo->file[strlen(cinfo->file) - 1] = '\0';
        if (put(cfd, cinfo) != 0) {
            cinfo->status = -2;
            in_to_out(cfd);
            return;
        }
    } else if (!strncmp(cinfo->desc, "DELETE", 6)) {
        cinfo->command = DELETE;
        strcpy(cinfo->file, cinfo->desc + 7);
        cinfo->file[strlen(cinfo->file) - 1] = '\0';
    } else if (!strncmp(cinfo->desc, "GET", 3)) {
        cinfo->command = GET;
        strcpy(cinfo->file, cinfo->desc + 4);
        cinfo->file[strlen(cinfo->file) - 1] = '\0';
    } else {
        print_invalid_response();
        cinfo->status = -1;
        in_to_out(cfd);
        return;
    }
    cinfo->status = 1;
    in_to_out(cfd);
}

void run_client(int cfd) {
    struct client_info * cinfo = dictionary_get(clients, &cfd);
    // int status = cinfo->status;
    if (cinfo->status == 0) {
        get_command(cfd, cinfo);
    } else if (cinfo->status == 1) {
        exec_command(cfd, cinfo);
    } else {
        write_all_to_socket(cfd, "ERROR\n", 6);
        if (cinfo->status == -1) {
            write_all_to_socket(cfd, err_bad_request, strlen(err_bad_request));
        }

        if (cinfo->status == -2) {
            write_all_to_socket(cfd, err_no_such_file, strlen(err_no_such_file));
        }

        if (cinfo->status == -3) {
            write_all_to_socket(cfd, err_no_such_file, strlen(err_no_such_file));
        }

        epoll_ctl(efd, EPOLL_CTL_DEL, cfd, NULL);
        free(cinfo);
        dictionary_remove(clients, &cfd);
        shutdown(cfd, SHUT_RDWR);
        close(cfd);
    }
}

void run(char * port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct addrinfo ainfo, *res;
    memset(&ainfo, 0, sizeof(ainfo));
    ainfo.ai_family = AF_INET;
    ainfo.ai_socktype = SOCK_STREAM;
    ainfo.ai_flags = AI_PASSIVE;

    int error = getaddrinfo(NULL, port, &ainfo, &res);
    if (error != 0) {
        perror("could not get addrinfo");
        exit(1);
    }

    int optval = 1;

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) != 0) {
        perror("could not set sock opt reuseport");
        exit(1);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) != 0) {
        perror("could not set sock opt reuseaddr");
        exit(1);
    }

    if (bind(sock, res->ai_addr, res->ai_addrlen) != 0) {
        perror ("could not bind");
        exit(1);
    }

    if (listen(sock, 100) != 0) {
        perror ("could not listen");
        exit(1);
    }

    freeaddrinfo(res);

    // epoll time
    // use epoll and EPOLLONESHOT for edge triggered---
    struct epoll_event event;
    efd = epoll_create1(0);
    if (efd == -1) {
        perror("could not create epoll");
        exit(1);
    }
    event.events = EPOLLIN;
    // event.events = EPOLLONESHOT;
    event.data.fd = sock;
    epoll_ctl(efd, EPOLL_CTL_ADD, sock, &event);
    struct epoll_event arr[100];
    while (true) {
        int n = epoll_wait(efd, arr, 100, 10000);
        if (n == -1) {
            perror("error waiting for epoll");
            exit(1);
        }
        for (int i = 0; i < n; i++) {
            if (arr[i].data.fd == sock) {
                int cfd = accept(sock, NULL, NULL);
                if (cfd == -1) {
                    perror("error accepting client");
                    exit(1);
                }
                struct epoll_event event2;
                event2.events = EPOLLIN;
                event2.data.fd = cfd;
                epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &event2);
                struct client_info *cinfo;
                cinfo = calloc(1, sizeof(struct client_info));
                dictionary_set(clients, &cfd, cinfo);
            } else {
                run_client(arr[i].data.fd);
            }
        }
    }
}

void sigpipe(){}

int main(int argc, char **argv) {
    // good luck!
    if (argc != 2) {
        print_server_usage();
        exit(1);
    }

    struct sigaction sig;
    memset(&sig, '\0', sizeof(struct sigaction));
    sig.sa_handler = off;
    if (sigaction(SIGINT, &sig, NULL) != 0) {
        perror("did not shutdown");
        exit(-1);
    }

    signal(SIGPIPE, sigpipe);

    char temp[] = "XXXXXX";
    directory = mkdtemp(temp);
    print_temp_directory(directory);


    clients = int_to_shallow_dictionary_create();
    files = string_vector_create();
    files_sizes = string_to_unsigned_long_dictionary_create();

    char * port = argv[1];
    run(port);
}
