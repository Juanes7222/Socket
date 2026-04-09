#ifndef NETWORK_H
#define NETWORK_H

#include "types.h"

int  network_create_listening_socket(void);
void network_accept_new_connection(int listen_fd, ServerContext *ctx);
void network_trim_trailing_newlines(char *buffer, int *length);

#endif