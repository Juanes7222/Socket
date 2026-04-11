#include "chat_history.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define SENDER_SELF   "you"
#define SENDER_SERVER "server"

typedef struct {
    char timestamp[8];
    char sender[USERNAME_MAX_LEN];
    char text[BUFFER_SIZE];
} HistoryEntry;

static HistoryEntry    recorded_entries[MAX_HISTORY_ENTRIES];
static int             entry_count = 0;
static CRITICAL_SECTION history_lock;


static void get_current_timestamp(char *buffer, size_t size)
{
    time_t    now   = time(NULL);
    struct tm *local = localtime(&now);
    strftime(buffer, size, "%H:%M", local);
}

static void get_full_datetime(char *buffer, size_t size)
{
    time_t    now   = time(NULL);
    struct tm *local = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", local);
}

static void strip_trailing_newlines(char *text)
{
    int len = (int)strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[--len] = '\0';
    }
}

static void append_entry(const char *sender, const char *text)
{
    if (entry_count >= MAX_HISTORY_ENTRIES) return;

    HistoryEntry *entry = &recorded_entries[entry_count];

    get_current_timestamp(entry->timestamp, sizeof(entry->timestamp));

    strncpy(entry->sender, sender, USERNAME_MAX_LEN - 1);
    entry->sender[USERNAME_MAX_LEN - 1] = '\0';

    strncpy(entry->text, text, BUFFER_SIZE - 1);
    entry->text[BUFFER_SIZE - 1] = '\0';
    strip_trailing_newlines(entry->text);

    entry_count++;
}

/*
 * Detects whether a raw message matches the peer chat pattern "username: body".
 * On match, fills sender_out and body_out and returns 1.
 * Returns 0 for system/server messages.
 */
static int extract_peer_message_parts(const char *raw,
                                       char       *sender_out,
                                       char       *body_out)
{
    const char *colon      = strstr(raw, ": ");
    if (colon == NULL) return 0;

    int prefix_len = (int)(colon - raw);
    if (prefix_len <= 0 || prefix_len >= USERNAME_MAX_LEN) return 0;

    for (int i = 0; i < prefix_len; i++) {
        char c = raw[i];
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') return 0;
    }

    strncpy(sender_out, raw, prefix_len);
    sender_out[prefix_len] = '\0';

    strncpy(body_out, colon + 2, BUFFER_SIZE - 1);
    body_out[BUFFER_SIZE - 1] = '\0';

    return 1;
}


void chat_history_init(void)
{
    entry_count = 0;
    memset(recorded_entries, 0, sizeof(recorded_entries));
    InitializeCriticalSection(&history_lock);
}

void chat_history_record_received(const char *raw_message)
{
    char sender[USERNAME_MAX_LEN];
    char body[BUFFER_SIZE];

    EnterCriticalSection(&history_lock);

    if (extract_peer_message_parts(raw_message, sender, body)) {
        append_entry(sender, body);
    } else {
        append_entry(SENDER_SERVER, raw_message);
    }

    LeaveCriticalSection(&history_lock);
}

void chat_history_record_sent(const char *text)
{
    EnterCriticalSection(&history_lock);
    append_entry(SENDER_SELF, text);
    LeaveCriticalSection(&history_lock);
}

int chat_history_entry_count(void)
{
    return entry_count;
}

int chat_history_export(const char *filepath)
{
    FILE *output_file = fopen(filepath, "w");
    if (output_file == NULL) return -1;

    char session_datetime[32];
    get_full_datetime(session_datetime, sizeof(session_datetime));

    fprintf(output_file, "=== Historial de Chat ===\n");
    fprintf(output_file, "Exportado : %s\n", session_datetime);
    fprintf(output_file, "Servidor  : %s:%d\n", SERVER_IP, SERVER_PORT);
    fprintf(output_file, "========================\n\n");

    EnterCriticalSection(&history_lock);

    for (int i = 0; i < entry_count; i++) {
        fprintf(output_file, "[%s] %s: %s\n",
                recorded_entries[i].timestamp,
                recorded_entries[i].sender,
                recorded_entries[i].text);
    }

    fprintf(output_file, "\n========================\n");
    fprintf(output_file, "Total: %d mensaje(s)\n", entry_count);

    LeaveCriticalSection(&history_lock);

    fclose(output_file);
    return 0;
}