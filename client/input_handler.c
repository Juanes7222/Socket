#include "input_handler.h"
#include "types.h"
#include <stdio.h>
#include <string.h>

static int is_exit_command(const char *input)
{
    return strncmp(input, "/exit", 5) == 0;
}

void input_handler_print_commands(void)
{
    printf("Available commands:\n");
    printf("  /list  - See all connected users and their current status\n");
    printf("  /exit  - Leave the current chat or disconnect from server\n\n");
}

void input_handler_run_loop(SOCKET server_socket)
{
    char input_buffer[BUFFER_SIZE];

    while (1) {
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) break;

        send(server_socket, input_buffer, strlen(input_buffer), 0);

        if (is_exit_command(input_buffer)) break;
    }
}