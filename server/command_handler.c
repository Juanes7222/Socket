#include "command_handler.h"
#include "session.h"
#include "room_session.h"
#include "messaging.h"
#include <string.h>

static int is_join_room_command(const char *message)
{
    return message[0] == '#';
}

static void handle_choosing_peer_input(int client_index, const char *message,
                                        ServerContext *ctx)
{
    if (strcmp(message, "/list") == 0) {
        messaging_send_connected_users_list(client_index, ctx);
        return;
    }

    if (strcmp(message, "/rooms") == 0) {
        messaging_send_room_list(client_index, ctx);
        return;
    }

    if (is_join_room_command(message)) {
        room_session_join(client_index, message + 1, ctx);
        return;
    }

    int peer_index = session_find_available_peer(message, client_index, ctx);
    if (peer_index >= 0) {
        session_pair_clients(client_index, peer_index, ctx);
    } else {
        messaging_send_raw(ctx->clients[client_index].fd,
                           "User not available. Choose another:\n");
        messaging_send_available_peers(client_index, ctx);
    }
}

static void handle_in_chat_input(int client_index, const char *message,
                                  ServerContext *ctx)
{
    if (strcmp(message, "/exit") == 0) {
        session_end_chat(client_index, ctx);
        return;
    }

    if (strcmp(message, "/list") == 0) {
        messaging_send_connected_users_list(client_index, ctx);
        return;
    }

    messaging_forward_chat_message(client_index, message, ctx);
}

static void handle_in_room_input(int client_index, const char *message,
                                  ServerContext *ctx)
{
    if (strcmp(message, "/leave") == 0) {
        room_session_leave(client_index, ctx);
        return;
    }

    if (strcmp(message, "/list") == 0) {
        messaging_send_connected_users_list(client_index, ctx);
        return;
    }

    if (strcmp(message, "/rooms") == 0) {
        messaging_send_room_list(client_index, ctx);
        return;
    }

    room_session_broadcast_message(client_index, message, ctx);
}

void command_handler_process(int client_index, const char *message,
                              ServerContext *ctx)
{
    switch (ctx->clients[client_index].state) {
        case STATE_AWAITING_NAME:
            session_register_username(client_index, message, ctx);
            break;

        case STATE_CHOOSING_PEER:
            handle_choosing_peer_input(client_index, message, ctx);
            break;

        case STATE_IN_CHAT:
            handle_in_chat_input(client_index, message, ctx);
            break;

        case STATE_IN_ROOM:
            handle_in_room_input(client_index, message, ctx);
            break;

        default:
            break;
    }
}