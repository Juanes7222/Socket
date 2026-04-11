#include "chat_ui.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define HEADER_LINE \
    "================================================================"

static CRITICAL_SECTION print_lock;

/* ── Private helpers ──────────────────────────────────────────────────────── */

static void enable_virtual_terminal_processing(void)
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  mode   = 0;

    if (GetConsoleMode(handle, &mode)) {
        SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}

static void get_timestamp(char *buffer, size_t size)
{
    time_t    now   = time(NULL);
    struct tm *local = localtime(&now);
    strftime(buffer, size, "%H:%M", local);
}

static int is_peer_chat_message(const char *text)
{
    const char *colon     = strstr(text, ": ");
    if (colon == NULL) return 0;

    int prefix_len = (int)(colon - text);
    if (prefix_len <= 0 || prefix_len >= USERNAME_MAX_LEN) return 0;

    for (int i = 0; i < prefix_len; i++) {
        char c = text[i];
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') return 0;
    }
    return 1;
}

/* ── Public interface ─────────────────────────────────────────────────────── */

void chat_ui_init(void)
{
    enable_virtual_terminal_processing();
    InitializeCriticalSection(&print_lock);

    printf("\033[2J\033[H");
    printf(COLOR_CYAN COLOR_BOLD HEADER_LINE COLOR_RESET "\n");
    printf(COLOR_CYAN COLOR_BOLD
           "  CHAT  |  Servidor: %s:%d"
           COLOR_RESET "\n",
           SERVER_IP, SERVER_PORT);
    printf(COLOR_CYAN COLOR_BOLD HEADER_LINE COLOR_RESET "\n\n");
    fflush(stdout);
}

void chat_ui_clear_prompt_line(void)
{
    EnterCriticalSection(&print_lock);
    printf("\r\033[2K");
    fflush(stdout);
    LeaveCriticalSection(&print_lock);
}

void chat_ui_show_prompt(void)
{
    EnterCriticalSection(&print_lock);
    printf(COLOR_CYAN " > " COLOR_WHITE);
    fflush(stdout);
    LeaveCriticalSection(&print_lock);
}

void chat_ui_print_received(const char *text)
{
    char timestamp[8];
    get_timestamp(timestamp, sizeof(timestamp));

    EnterCriticalSection(&print_lock);

    if (is_peer_chat_message(text)) {
        const char *colon    = strstr(text, ": ");
        int         name_len = (int)(colon - text);
        char        name[USERNAME_MAX_LEN];

        strncpy(name, text, name_len);
        name[name_len] = '\0';

        printf(COLOR_GRAY " [%s]" COLOR_RESET " "
               COLOR_GREEN COLOR_BOLD "%s" COLOR_RESET
               ": " COLOR_WHITE "%s" COLOR_RESET,
               timestamp, name, colon + 2);
    } else {
        printf(COLOR_GRAY " [%s]" COLOR_RESET " "
               COLOR_YELLOW "%s" COLOR_RESET,
               timestamp, text);
    }

    size_t len = strlen(text);
    if (len > 0 && text[len - 1] != '\n') printf("\n");

    fflush(stdout);
    LeaveCriticalSection(&print_lock);
}

/*
 * Prints a client-side notice (not from the server) in a distinct style so
 * the user can tell it apart from server messages.
 */
void chat_ui_print_notice(const char *text)
{
    EnterCriticalSection(&print_lock);
    printf(COLOR_CYAN " [*] " COLOR_RESET COLOR_DIM "%s" COLOR_RESET "\n", text);
    fflush(stdout);
    LeaveCriticalSection(&print_lock);
}

void chat_ui_print_disconnect_notice(void)
{
    EnterCriticalSection(&print_lock);
    printf("\n" COLOR_YELLOW
           " [!] La conexion con el servidor fue cerrada."
           COLOR_RESET "\n");
    fflush(stdout);
    LeaveCriticalSection(&print_lock);
}