#ifndef SESSION_H
#define SESSION_H

#include "types.h"

void session_register_username(int client_index, const char *username,
                                ServerContext *ctx);
void session_pair_clients(int initiator_index, int target_index,
                           ServerContext *ctx);
void session_end_chat(int client_index, ServerContext *ctx);
void session_disconnect_client(int client_index, ServerContext *ctx);
int  session_find_available_peer(const char *target_username,
                                  int requester_index,
                                  const ServerContext *ctx);

#endif