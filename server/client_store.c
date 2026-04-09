#include "client_store.h"

void client_store_init(ServerContext *ctx)
{
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        ctx->fds[i].fd               = -1;
        ctx->clients[i].fd           = -1;
        ctx->clients[i].state        = STATE_EMPTY;
        ctx->clients[i].peer_index   = -1;
        ctx->clients[i].username[0]  = '\0';
    }
}

int client_store_find_free_slot(const ServerContext *ctx)
{
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (ctx->fds[i].fd < 0) return i;
    }
    return -1;
}

void client_store_assign_slot(int slot, int conn_fd, ServerContext *ctx)
{
    ctx->fds[slot].fd              = conn_fd;
    ctx->fds[slot].events          = POLLIN;
    ctx->clients[slot].fd          = conn_fd;
    ctx->clients[slot].state       = STATE_AWAITING_NAME;
    ctx->clients[slot].peer_index  = -1;
    ctx->clients[slot].username[0] = '\0';
}

void client_store_clear_slot(int slot, ServerContext *ctx)
{
    ctx->fds[slot].fd              = -1;
    ctx->clients[slot].fd          = -1;
    ctx->clients[slot].state       = STATE_EMPTY;
    ctx->clients[slot].peer_index  = -1;
    ctx->clients[slot].username[0] = '\0';
}