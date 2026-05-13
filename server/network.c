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
    int listen_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                   &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    int ipv6only = 0;
    if (setsockopt(listen_fd, IPPROTO_IPV6, IPV6_V6ONLY,
                   &ipv6only, sizeof(ipv6only)) < 0) {
        perror("setsockopt IPV6_V6ONLY");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 address = {0};
    address.sin6_family = AF_INET6;
    address.sin6_port   = htons(SERVER_PORT);
    address.sin6_addr   = in6addr_any;

    if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d (IPv4 + IPv6 dual-stack)\n",
           SERVER_PORT);
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