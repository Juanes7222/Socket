#ifndef CHAT_HISTORY_H
#define CHAT_HISTORY_H

#define MAX_HISTORY_ENTRIES 500

void chat_history_init(void);
void chat_history_record_received(const char *raw_message);
void chat_history_record_sent(const char *text);
int  chat_history_export(const char *filepath);
int  chat_history_entry_count(void);

#endif