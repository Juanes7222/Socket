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
        "Available commands at any time: /list, /exit\n"
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
        if (ctx->clients[i].fd >= 0 && ctx->clients[i].state == STATE_CHOOSING_PEER) {
            written += snprintf(response + written, sizeof(response) - written,
                                "  - %s\n", ctx->clients[i].username);
        }
    }

    written += snprintf(response + written, sizeof(response) - written,
                        "Type a username to start chatting"
                        " (or /list to see all users):\n");

    send(ctx->clients[requester_index].fd, response, written, 0);
}

void messaging_send_connected_users_list(int requester_index, const ServerContext *ctx)
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
                                "  - (unregistered user)\n");
        } else {
            written += snprintf(response + written, sizeof(response) - written,
                                "  - %s [%s]\n",
                                ctx->clients[i].username,
                                state_to_string(ctx->clients[i].state));
        }
    }

    written += snprintf(response + written, sizeof(response) - written,
                        "Total: %d user(s) connected\n"
                        "=======================\n\n",
                        user_count);

    send(ctx->clients[requester_index].fd, response, written, 0);
}

void messaging_broadcast_available_peers(ServerContext *ctx)
{
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (ctx->clients[i].fd >= 0 && ctx->clients[i].state == STATE_CHOOSING_PEER) {
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