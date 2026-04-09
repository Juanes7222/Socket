#include "winsock_setup.h"
#include <winsock2.h>
#include <stdio.h>

static WSADATA wsa_data;

int winsock_initialize(void)
{
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        printf("Failed to initialize Winsock: error code %d\n", result);
    }
    return result;
}

void winsock_cleanup(void)
{
    WSACleanup();
}