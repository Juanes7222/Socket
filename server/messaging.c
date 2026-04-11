#include "messaging.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

static const char *state_to_string(ClientState state)
{
    switch (state) {
        case STATE_AWAITING_NAME: return "registering";
        case STATE_CHOOSING_PEER: return "available";
        case STATE_IN_CHAT:       return "in chat";
        case STATE_IN_ROOM:       return "in room";
        default:                  return "unknown";
    }
}

void messaging_send_raw(int client_fd, const char *message)
{
    send(client_fd, message, strlen(message), 0);
}

void messaging_send_welcome(int client_fd)
{
    const char *welcome =
        "Welcome to the chat server.\n"
        "Commands: /list, /rooms, /exit\n"
        "To chat privately: type a username\n"
        "To join a room:    type #roomname\n\n"
        "Enter your username:\n";
    messaging_send_raw(client_fd, welcome);
}

void messaging_send_server_full(int client_fd)
{
    messaging_send_raw(client_fd, "Server is full. Try again later.\n");
}

void messaging_send_available_peers(int requester_index, const ServerContext *ctx)
{
    char response[BUFFER_SIZE];
    int  written = 0;

    written += snprintf(response + written, sizeof(response) - written,
                        "Available users:\n");

    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (i == requester_index) continue;
        if (ctx->clients[i].fd >= 0
            && ctx->clients[i].state == STATE_CHOOSING_PEER) {
            written += snprintf(response + written, sizeof(response) - written,
                                "  - %s\n", ctx->clients[i].username);
        }
    }

    written += snprintf(response + written, sizeof(response) - written,
                        "Type a username to chat or #roomname to join a room:\n");

    send(ctx->clients[requester_index].fd, response, written, 0);
}

void messaging_send_connected_users_list(int requester_index,
                                          const ServerContext *ctx)
{
    char response[BUFFER_SIZE];
    int  written    = 0;
    int  user_count = 0;

    written += snprintf(response + written, sizeof(response) - written,
                        "\n=== Connected users ===\n");

    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (ctx->clients[i].fd < 0) continue;
        user_count++;

        if (i == requester_index) {
            written += snprintf(response + written, sizeof(response) - written,
                                "  [you] %s [%s]\n",
                                ctx->clients[i].username,
                                state_to_string(ctx->clients[i].state));
        } else if (ctx->clients[i].state == STATE_AWAITING_NAME) {
            written += snprintf(response + written, sizeof(response) - written,
                                "  - (unregistered)\n");
        } else {
            written += snprintf(response + written, sizeof(response) - written,
                                "  - %s [%s]\n",
                                ctx->clients[i].username,
                                state_to_string(ctx->clients[i].state));
        }
    }

    written += snprintf(response + written, sizeof(response) - written,
                        "Total: %d user(s)\n"
                        "=======================\n\n",
                        user_count);

    send(ctx->clients[requester_index].fd, response, written, 0);
}

void messaging_send_room_list(int requester_index, const ServerContext *ctx)
{
    char response[BUFFER_SIZE];
    int  written    = 0;
    int  room_count = 0;

    written += snprintf(response + written, sizeof(response) - written,
                        "\n=== Active rooms ===\n");

    for (int i = 0; i < MAX_ROOMS; i++) {
        if (!ctx->rooms[i].is_active) continue;
        room_count++;
        written += snprintf(response + written, sizeof(response) - written,
                            "  #%-20s  (%d member(s))\n",
                            ctx->rooms[i].room_name,
                            ctx->rooms[i].member_count);
    }

    if (room_count == 0) {
        written += snprintf(response + written, sizeof(response) - written,
                            "  No active rooms. Create one with #roomname\n");
    }

    written += snprintf(response + written, sizeof(response) - written,
                        "====================\n\n");

    send(ctx->clients[requester_index].fd, response, written, 0);
}

void messaging_broadcast_available_peers(ServerContext *ctx)
{
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (ctx->clients[i].fd >= 0
            && ctx->clients[i].state == STATE_CHOOSING_PEER) {
            messaging_send_available_peers(i, ctx);
        }
    }
}

void messaging_forward_chat_message(int sender_index, const char *text,
                                     const ServerContext *ctx)
{
    int peer_index = ctx->clients[sender_index].peer_index;
    if (peer_index <= 0 || ctx->clients[peer_index].fd < 0) return;

    char formatted[BUFFER_SIZE];
    int  length = snprintf(formatted, sizeof(formatted),
                           "%s: %s\n",
                           ctx->clients[sender_index].username, text);

    send(ctx->clients[peer_index].fd, formatted, length, 0);
}

void messaging_send_room_message(int recipient_fd, const char *room_name,
                                  const char *sender_username, const char *text)
{
    char formatted[BUFFER_SIZE];
    int  length = snprintf(formatted, sizeof(formatted),
                           "[#%s] %s: %s\n",
                           room_name, sender_username, text);
    send(recipient_fd, formatted, length, 0);
}

void messaging_notify_room_join(int room_index, int new_member_index,
                                 const ServerContext *ctx)
{
    const ChatRoom *room       = &ctx->rooms[room_index];
    const char     *new_member = ctx->clients[new_member_index].username;

    char welcome_message[BUFFER_SIZE];
    snprintf(welcome_message, sizeof(welcome_message),
             "Joined #%s (%d member(s)). Commands: /leave, /list, /rooms\n",
             room->room_name, room->member_count);
    messaging_send_raw(ctx->clients[new_member_index].fd, welcome_message);

    char join_notification[BUFFER_SIZE];
    snprintf(join_notification, sizeof(join_notification),
             "[#%s] %s joined the room.\n", room->room_name, new_member);

    for (int i = 0; i <= MAX_CLIENTS; i++) {
        int member_index = room->member_indices[i];
        if (member_index < 0 || member_index == new_member_index) continue;
        if (ctx->clients[member_index].fd < 0) continue;
        messaging_send_raw(ctx->clients[member_index].fd, join_notification);
    }
}

void messaging_notify_room_leave(int room_index, int leaving_member_index,
                                  const ServerContext *ctx)
{
    const ChatRoom *room           = &ctx->rooms[room_index];
    const char     *leaving_member =
        ctx->clients[leaving_member_index].username;

    char leave_notification[BUFFER_SIZE];
    snprintf(leave_notification, sizeof(leave_notification),
             "[#%s] %s left the room.\n", room->room_name, leaving_member);

    for (int i = 0; i <= MAX_CLIENTS; i++) {
        int member_index = room->member_indices[i];
        if (member_index < 0 || member_index == leaving_member_index) continue;
        if (ctx->clients[member_index].fd < 0) continue;
        messaging_send_raw(ctx->clients[member_index].fd, leave_notification);
    }
}