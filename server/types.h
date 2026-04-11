#ifndef TYPES_H
#define TYPES_H

#include <poll.h>

#define SERVER_PORT       8080
#define BUFFER_SIZE       1024
#define MAX_CLIENTS       100
#define USERNAME_MAX_LEN  32
#define MAX_ROOMS         10
#define ROOM_NAME_MAX     32

typedef enum {
    STATE_AWAITING_NAME = 0,
    STATE_CHOOSING_PEER = 1,
    STATE_IN_CHAT       = 2,
    STATE_IN_ROOM       = 3,
    STATE_EMPTY         = -1
} ClientState;

typedef struct {
    char room_name[ROOM_NAME_MAX];
    int  member_indices[MAX_CLIENTS + 1];
    int  member_count;
    int  is_active;
} ChatRoom;

typedef struct {
    int         fd;
    char        username[USERNAME_MAX_LEN];
    ClientState state;
    int         peer_index;
    int         room_index;
} Client;

typedef struct {
    struct pollfd fds[MAX_CLIENTS + 1];
    Client        clients[MAX_CLIENTS + 1];
    ChatRoom      rooms[MAX_ROOMS];
} ServerContext;

#endif