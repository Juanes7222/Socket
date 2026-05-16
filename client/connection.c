#include "connection.h"
#include "chat_ui.h"
#include "types.h"
#include <stdio.h>
#include <ws2tcpip.h>

SOCKET connection_create_and_connect(void)
{
    struct addrinfo hints = {0};
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    SOCKET sock = INVALID_SOCKET;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", SERVER_PORT);

    int iResult = getaddrinfo(SERVER_IP, port_str, &hints, &result);
    if (iResult != 0) {
        printf(COLOR_YELLOW " [!] getaddrinfo failed: %d\n" COLOR_RESET, iResult);
        return INVALID_SOCKET;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET) {
            continue;
        }

        if (connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (sock == INVALID_SOCKET) {
        printf(COLOR_YELLOW " [!] No se pudo conectar a %s:%d (Error: %d)\n" COLOR_RESET, SERVER_IP, SERVER_PORT, WSAGetLastError());
        return INVALID_SOCKET;
    }

    printf(COLOR_CYAN " Conectado a %s:%d\n\n" COLOR_RESET, SERVER_IP, SERVER_PORT);
    return sock;
}