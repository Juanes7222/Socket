#include "input_handler.h"
#include "chat_ui.h"
#include "chat_history.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static int is_exit_command(const char *input)
{
    return strncmp(input, "/exit", 5) == 0;
}

static int is_save_command(const char *input)
{
    return strncmp(input, "/save", 5) == 0;
}

static int is_any_command(const char *input)
{
    return input[0] == '/';
}

static void generate_export_filename(char *buffer, size_t size)
{
    time_t    now   = time(NULL);
    struct tm *local = localtime(&now);
    strftime(buffer, size, "chat_%Y%m%d_%H%M%S.txt", local);
}

static void export_history_with_feedback(void)
{
    if (chat_history_entry_count() == 0) {
        chat_ui_clear_prompt_line();
        chat_ui_print_notice("No hay mensajes en el historial para exportar.");
        return;
    }

    char filename[64];
    generate_export_filename(filename, sizeof(filename));

    int export_result = chat_history_export(filename);

    chat_ui_clear_prompt_line();

    if (export_result == 0) {
        char notice[128];
        snprintf(notice, sizeof(notice),
                 "Historial exportado a: %s (%d mensaje(s))",
                 filename, chat_history_entry_count());
        chat_ui_print_notice(notice);
    } else {
        chat_ui_print_notice("Error: no se pudo crear el archivo de historial.");
    }
}

void input_handler_print_commands(void)
{
    printf(COLOR_GRAY
           "  Comandos: "
           COLOR_CYAN "/list  /rooms  /leave  /save  /exit\n"
           COLOR_RESET
           COLOR_GRAY
           "  Para chatear: "
           COLOR_WHITE "nombre_usuario\n"
           COLOR_GRAY
           "  Para entrar a una sala: "
           COLOR_MAGENTA "#nombre_sala\n"
           COLOR_RESET "\n");
    fflush(stdout);
}

void input_handler_run_loop(SOCKET server_socket)
{
    char input_buffer[BUFFER_SIZE];

    chat_ui_show_prompt();

    while (1) {
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) break;

        if (is_save_command(input_buffer)) {
            export_history_with_feedback();
            chat_ui_show_prompt();
            continue;
        }

        send(server_socket, input_buffer, strlen(input_buffer), 0);

        if (is_exit_command(input_buffer)) {
            export_history_with_feedback();
            break;
        }

        if (!is_any_command(input_buffer)) {
            chat_history_record_sent(input_buffer);
        }

        chat_ui_show_prompt();
    }
}