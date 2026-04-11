#include "connection.h"
#include "chat_ui.h"
#include "types.h"
#include <stdio.h>

SOCKET connection_create_and_connect(void)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf(COLOR_YELLOW " [!] No se pudo crear el socket: error %d\n"
               COLOR_RESET, WSAGetLastError());
        return INVALID_SOCKET;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        printf(COLOR_YELLOW " [!] No se pudo conectar a %s:%d\n"
               COLOR_RESET, SERVER_IP, SERVER_PORT);
        closesocket(sock);
        return INVALID_SOCKET;
    }

    printf(COLOR_CYAN " Conectado a %s:%d\n\n" COLOR_RESET,
           SERVER_IP, SERVER_PORT);
    return sock;
}