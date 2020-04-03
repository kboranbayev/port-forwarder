#ifndef HANDLERS_H
#define HANDLERS_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#include "constants.h"

void DieWithError (char *error);

void traffic (int src, int dst);

int open_forwarding_socket (char *forward_name, int forward_port);

void handle_forward (int client_socket, char *forward_name, int forward_port);

int getLineCount (char *file);

struct IP_PORT * parseFile (char *file, int maxLine);

#endif


