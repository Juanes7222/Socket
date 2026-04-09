#ifndef RECEIVER_H
#define RECEIVER_H

#include <winsock2.h>
#include <windows.h>

DWORD WINAPI receiver_thread(LPVOID socket_param);

#endif