#include "room_store.h"
#include <stdio.h>
#include <string.h>

static int find_free_room_slot(const ServerContext *ctx)
{
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (!ctx->rooms[i].is_active) return i;
    }
    return -1;
}

void room_store_init(ServerContext *ctx)
{
    for (int i = 0; i < MAX_ROOMS; i++) {
        ctx->rooms[i].is_active    = 0;
        ctx->rooms[i].member_count = 0;
        ctx->rooms[i].room_name[0] = '\0';
        for (int j = 0; j <= MAX_CLIENTS; j++) {
            ctx->rooms[i].member_indices[j] = -1;
        }
    }
}

int room_store_find_by_name(const char *room_name, const ServerContext *ctx)
{
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (ctx->rooms[i].is_active
            && strcmp(ctx->rooms[i].room_name, room_name) == 0) {
            return i;
        }
    }
    return -1;
}

int room_store_find_or_create(const char *room_name, ServerContext *ctx)
{
    int existing_index = room_store_find_by_name(room_name, ctx);
    if (existing_index >= 0) return existing_index;

    int new_slot = find_free_room_slot(ctx);
    if (new_slot < 0) return -1;

    strncpy(ctx->rooms[new_slot].room_name, room_name, ROOM_NAME_MAX - 1);
    ctx->rooms[new_slot].room_name[ROOM_NAME_MAX - 1] = '\0';
    ctx->rooms[new_slot].is_active    = 1;
    ctx->rooms[new_slot].member_count = 0;

    for (int j = 0; j <= MAX_CLIENTS; j++) {
        ctx->rooms[new_slot].member_indices[j] = -1;
    }

    printf("Room '#%s' created at slot %d\n", room_name, new_slot);
    return new_slot;
}

void room_store_add_member(int room_index, int client_index, ServerContext *ctx)
{
    ChatRoom *room = &ctx->rooms[room_index];
    for (int i = 0; i <= MAX_CLIENTS; i++) {
        if (room->member_indices[i] < 0) {
            room->member_indices[i] = client_index;
            room->member_count++;
            return;
        }
    }
}

void room_store_remove_member(int room_index, int client_index, ServerContext *ctx)
{
    ChatRoom *room = &ctx->rooms[room_index];
    for (int i = 0; i <= MAX_CLIENTS; i++) {
        if (room->member_indices[i] == client_index) {
            room->member_indices[i] = -1;
            room->member_count--;
            return;
        }
    }
}

void room_store_deactivate_if_empty(int room_index, ServerContext *ctx)
{
    if (ctx->rooms[room_index].member_count > 0) return;

    printf("Room '#%s' deactivated (no members left)\n",
           ctx->rooms[room_index].room_name);

    ctx->rooms[room_index].is_active    = 0;
    ctx->rooms[room_index].member_count = 0;
    ctx->rooms[room_index].room_name[0] = '\0';
}