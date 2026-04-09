#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "types.h"

void command_handler_process(int client_index, const char *message,
                              ServerContext *ctx);

#endif