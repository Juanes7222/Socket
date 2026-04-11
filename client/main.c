#include <winsock2.h>
#include <windows.h>
#include "winsock_setup.h"
#include "connection.h"
#include "receiver.h"
#include "input_handler.h"
#include "chat_ui.h"

int main(void)
{
    chat_ui_init();

    if (winsock_initialize() != 0) return 1;

    SOCKET server_socket = connection_create_and_connect();
    if (server_socket == INVALID_SOCKET) {
        winsock_cleanup();
        return 1;
    }

    input_handler_print_commands();

    DWORD receiver_thread_id;
    CreateThread(NULL, 0, receiver_thread,
                 &server_socket, 0, &receiver_thread_id);

    input_handler_run_loop(server_socket);

    closesocket(server_socket);
    winsock_cleanup();
    return 0;
}