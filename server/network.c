#include "network.h"
#include "client_store.h"
#include "messaging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int network_create_listening_socket(void)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int reuse_addr = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
               &reuse_addr, sizeof(reuse_addr));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(SERVER_PORT);

    if (bind(listen_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 5) < 0) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", SERVER_PORT);
    return listen_fd;
}

void network_accept_new_connection(int listen_fd, ServerContext *ctx)
{
    int conn_fd = accept(listen_fd, NULL, NULL);
    if (conn_fd < 0) {
        perror("accept");
        return;
    }

    int slot = client_store_find_free_slot(ctx);
    if (slot < 0) {
        messaging_send_server_full(conn_fd);
        close(conn_fd);
        return;
    }

    client_store_assign_slot(slot, conn_fd, ctx);
    messaging_send_welcome(conn_fd);
    printf("New connection fd=%d assigned to slot %d\n", conn_fd, slot);
}

void network_trim_trailing_newlines(char *buffer, int *length)
{
    while (*length > 0
           && (buffer[*length - 1] == '\n' || buffer[*length - 1] == '\r')) {
        buffer[--(*length)] = '\0';
    }
}