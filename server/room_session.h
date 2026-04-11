#ifndef ROOM_SESSION_H
#define ROOM_SESSION_H

#include "types.h"

void room_session_join(int client_index, const char *room_name,
                        ServerContext *ctx);
void room_session_leave(int client_index, ServerContext *ctx);
void room_session_broadcast_message(int sender_index, const char *text,
                                     ServerContext *ctx);

#endif