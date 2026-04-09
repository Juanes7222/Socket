#ifndef CLIENT_STORE_H
#define CLIENT_STORE_H

#include "types.h"

void client_store_init(ServerContext *ctx);
int  client_store_find_free_slot(const ServerContext *ctx);
void client_store_assign_slot(int slot, int conn_fd, ServerContext *ctx);
void client_store_clear_slot(int slot, ServerContext *ctx);

#endif