#include "receiver.h"
#include "chat_ui.h"
#include "chat_history.h"
#include "types.h"

DWORD WINAPI receiver_thread(LPVOID socket_param)
{
    SOCKET server_socket = *((SOCKET *)socket_param);
    char   buffer[BUFFER_SIZE];
    int    bytes_received;

    while ((bytes_received = recv(server_socket,
                                  buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';

        chat_ui_clear_prompt_line();
        chat_ui_print_received(buffer);
        chat_history_record_received(buffer);
        chat_ui_show_prompt();
    }

    chat_ui_print_disconnect_notice();
    return 0;
}