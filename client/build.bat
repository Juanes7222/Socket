@echo off
gcc main.c winsock_setup.c connection.c receiver.c input_handler.c ^
    chat_ui.c chat_history.c ^
    -o chat_client.exe -lws2_32
echo Build complete.
