#include "input_handler.h"
#include "chat_ui.h"
#include "types.h"
#include <stdio.h>
#include <string.h>

static int is_exit_command(const char *input)
{
    return strncmp(input, "/exit", 5) == 0;
}

void input_handler_print_commands(void)
{
    printf(COLOR_GRAY
           "  Comandos disponibles: "
           COLOR_CYAN "/list" COLOR_GRAY "  "
           COLOR_CYAN "/exit\n"
           COLOR_RESET "\n");
    fflush(stdout);
}

void input_handler_run_loop(SOCKET server_socket)
{
    char input_buffer[BUFFER_SIZE];

    chat_ui_show_prompt();

    while (1) {
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) break;

        send(server_socket, input_buffer, strlen(input_buffer), 0);

        if (is_exit_command(input_buffer)) break;

        chat_ui_show_prompt();
    }
}