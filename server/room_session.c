#include "room_session.h"
#include "room_store.h"
#include "messaging.h"
#include <stdio.h>
#include <string.h>

void room_session_join(int client_index, const char *room_name,
                        ServerContext *ctx)
{
    if (strlen(room_name) == 0) {
        messaging_send_raw(ctx->clients[client_index].fd,
                           "Room name cannot be empty. Use: #roomname\n");
        return;
    }

    int room_index = room_store_find_or_create(room_name, ctx);
    if (room_index < 0) {
        messaging_send_raw(ctx->clients[client_index].fd,
                           "No room slots available. Try again later.\n");
        return;
    }

    ctx->clients[client_index].state      = STATE_IN_ROOM;
    ctx->clients[client_index].room_index = room_index;

    room_store_add_member(room_index, client_index, ctx);
    messaging_notify_room_join(room_index, client_index, ctx);

    printf("'%s' joined room '#%s'\n",
           ctx->clients[client_index].username, room_name);
}

void room_session_leave(int client_index, ServerContext *ctx)
{
    int  room_index = ctx->clients[client_index].room_index;
    char room_name_copy[ROOM_NAME_MAX];

    strncpy(room_name_copy, ctx->rooms[room_index].room_name, ROOM_NAME_MAX - 1);
    room_name_copy[ROOM_NAME_MAX - 1] = '\0';

    room_store_remove_member(room_index, client_index, ctx);

    ctx->clients[client_index].state      = STATE_CHOOSING_PEER;
    ctx->clients[client_index].room_index = -1;

    messaging_notify_room_leave(room_index, client_index, ctx);
    room_store_deactivate_if_empty(room_index, ctx);

    messaging_send_raw(ctx->clients[client_index].fd, "You left the room.\n");
    messaging_send_available_peers(client_index, ctx);

    printf("'%s' left room '#%s'\n",
           ctx->clients[client_index].username, room_name_copy);
}

void room_session_broadcast_message(int sender_index, const char *text,
                                     ServerContext *ctx)
{
    int              room_index = ctx->clients[sender_index].room_index;
    const ChatRoom  *room       = &ctx->rooms[room_index];

    for (int i = 0; i <= MAX_CLIENTS; i++) {
        int member_index = room->member_indices[i];
        if (member_index < 0 || member_index == sender_index) continue;
        if (ctx->clients[member_index].fd < 0) continue;

        messaging_send_room_message(ctx->clients[member_index].fd,
                                     room->room_name,
                                     ctx->clients[sender_index].username,
                                     text);
    }
}