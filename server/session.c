#include "session.h"
#include "messaging.h"
#include "client_store.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void session_register_username(int client_index, const char *username,
                                ServerContext *ctx)
{
    strncpy(ctx->clients[client_index].username, username, USERNAME_MAX_LEN - 1);
    ctx->clients[client_index].username[USERNAME_MAX_LEN - 1] = '\0';
    ctx->clients[client_index].state = STATE_CHOOSING_PEER;

    printf("Client fd=%d registered as '%s'\n",
           ctx->clients[client_index].fd,
           ctx->clients[client_index].username);

    // Refresh peer list for the new client and all others already waiting.
    messaging_broadcast_available_peers(ctx);
}

void session_pair_clients(int initiator_index, int target_index,
                           ServerContext *ctx)
{
    ctx->clients[initiator_index].peer_index = target_index;
    ctx->clients[target_index].peer_index    = initiator_index;
    ctx->clients[initiator_index].state      = STATE_IN_CHAT;
    ctx->clients[target_index].state         = STATE_IN_CHAT;

    const char *start_message =
        "Chat started. Available commands:\n"
        "  /exit  - Leave the chat\n"
        "  /list  - See all connected users\n\n";

    messaging_send_raw(ctx->clients[initiator_index].fd, start_message);
    messaging_send_raw(ctx->clients[target_index].fd,    start_message);

    printf("Paired '%s' <--> '%s'\n",
           ctx->clients[initiator_index].username,
           ctx->clients[target_index].username);
}

void session_end_chat(int client_index, ServerContext *ctx)
{
    int peer_index = ctx->clients[client_index].peer_index;

    messaging_send_raw(ctx->clients[client_index].fd, "You left the chat.\n");

    if (peer_index > 0) {
        messaging_send_raw(ctx->clients[peer_index].fd,
                           "The other user left the chat.\n");
        ctx->clients[peer_index].state      = STATE_CHOOSING_PEER;
        ctx->clients[peer_index].peer_index = -1;
        messaging_send_available_peers(peer_index, ctx);
    }

    ctx->clients[client_index].state      = STATE_CHOOSING_PEER;
    ctx->clients[client_index].peer_index = -1;
    messaging_send_available_peers(client_index, ctx);

    printf("Chat ended for '%s'\n", ctx->clients[client_index].username);
}

void session_disconnect_client(int client_index, ServerContext *ctx)
{
    printf("Client '%s' (fd=%d) disconnected\n",
           ctx->clients[client_index].username,
           ctx->clients[client_index].fd);

    int peer_index = ctx->clients[client_index].peer_index;

    if (peer_index > 0 && ctx->clients[peer_index].state == STATE_IN_CHAT) {
        messaging_send_raw(ctx->clients[peer_index].fd,
                           "The other user disconnected.\n");
        ctx->clients[peer_index].state      = STATE_CHOOSING_PEER;
        ctx->clients[peer_index].peer_index = -1;
        messaging_send_available_peers(peer_index, ctx);
    }

    close(ctx->clients[client_index].fd);
    client_store_clear_slot(client_index, ctx);
}

int session_find_available_peer(const char *target_username,
                                 int requester_index,
                                 const ServerContext *ctx)
{
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (i == requester_index) continue;
        if (ctx->clients[i].fd >= 0
            && ctx->clients[i].state == STATE_CHOOSING_PEER
            && strcmp(ctx->clients[i].username, target_username) == 0) {
            return i;
        }
    }
    return -1;
}