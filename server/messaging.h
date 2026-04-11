#ifndef MESSAGING_H
#define MESSAGING_H

#include "types.h"

void messaging_send_welcome(int client_fd);
void messaging_send_server_full(int client_fd);
void messaging_send_raw(int client_fd, const char *message);
void messaging_send_available_peers(int requester_index, const ServerContext *ctx);
void messaging_send_connected_users_list(int requester_index, const ServerContext *ctx);
void messaging_send_room_list(int requester_index, const ServerContext *ctx);
void messaging_broadcast_available_peers(ServerContext *ctx);
void messaging_forward_chat_message(int sender_index, const char *text,
                                     const ServerContext *ctx);
void messaging_send_room_message(int recipient_fd, const char *room_name,
                                  const char *sender_username, const char *text);
void messaging_notify_room_join(int room_index, int new_member_index,
                                 const ServerContext *ctx);
void messaging_notify_room_leave(int room_index, int leaving_member_index,
                                  const ServerContext *ctx);

#endif