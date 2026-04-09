#include "receiver.h"
#include "types.h"
#include <stdio.h>

DWORD WINAPI receiver_thread(LPVOID socket_param)
{
    SOCKET server_socket = *((SOCKET *)socket_param);
    char   buffer[BUFFER_SIZE];
    int    bytes_received;

    while ((bytes_received = recv(server_socket,
                                  buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }

    printf("\n[Connection to server closed]\n");
    return 0;
}