#ifndef TYPES_H
#define TYPES_H

#include <poll.h>

#define SERVER_PORT       8080
#define BUFFER_SIZE       1024
#define MAX_CLIENTS       100
#define USERNAME_MAX_LEN  32

typedef enum {
    STATE_AWAITING_NAME = 0,
    STATE_CHOOSING_PEER = 1,
    STATE_IN_CHAT       = 2,
    STATE_EMPTY         = -1
} ClientState;

typedef struct {
    int         fd;
    char        username[USERNAME_MAX_LEN];
    ClientState state;
    int         peer_index;
} Client;

typedef struct {
    struct pollfd fds[MAX_CLIENTS + 1];
    Client        clients[MAX_CLIENTS + 1];
} ServerContext;

#endif