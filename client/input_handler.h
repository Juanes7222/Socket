#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <winsock2.h>

void input_handler_print_commands(void);
void input_handler_run_loop(SOCKET server_socket);

#endif