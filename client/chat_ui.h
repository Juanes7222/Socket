#ifndef CHAT_UI_H
#define CHAT_UI_H

/* ANSI color codes shared across all client modules */
#define COLOR_RESET   "\033[0m"
#define COLOR_DIM     "\033[2m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_GRAY    "\033[90m"
#define COLOR_CYAN    "\033[96m"
#define COLOR_GREEN   "\033[92m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_WHITE   "\033[97m"

void chat_ui_init(void);
void chat_ui_print_received(const char *text);
void chat_ui_print_disconnect_notice(void);
void chat_ui_clear_prompt_line(void);
void chat_ui_show_prompt(void);

#endif