#ifndef ROOM_STORE_H
#define ROOM_STORE_H

#include "types.h"

void room_store_init(ServerContext *ctx);
int  room_store_find_by_name(const char *room_name, const ServerContext *ctx);
int  room_store_find_or_create(const char *room_name, ServerContext *ctx);
void room_store_add_member(int room_index, int client_index, ServerContext *ctx);
void room_store_remove_member(int room_index, int client_index, ServerContext *ctx);
void room_store_deactivate_if_empty(int room_index, ServerContext *ctx);

#endif