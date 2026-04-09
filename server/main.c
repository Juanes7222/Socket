#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include "types.h"
#include "network.h"
#include "client_store.h"
#include "session.h"
#include "command_handler.h"

static void run_event_loop(int listen_fd, ServerContext *ctx)
{
    while (1) {
        int ready_count = poll(ctx->fds, MAX_CLIENTS + 1, -1);
        if (ready_count < 0) {
            perror("poll");
            break;
        }

        if (ctx->fds[0].revents & POLLIN) {
            network_accept_new_connection(listen_fd, ctx);
            if (--ready_count == 0) continue;
        }

        for (int i = 1; i <= MAX_CLIENTS; i++) {
            if (ctx->fds[i].fd < 0)              continue;
            if (!(ctx->fds[i].revents & POLLIN)) continue;

            char buffer[BUFFER_SIZE];
            int  bytes_received = recv(ctx->fds[i].fd,
                                       buffer, BUFFER_SIZE - 1, 0);

            if (bytes_received <= 0) {
                session_disconnect_client(i, ctx);
            } else {
                buffer[bytes_received] = '\0';
                network_trim_trailing_newlines(buffer, &bytes_received);
                command_handler_process(i, buffer, ctx);
            }

            if (--ready_count == 0) break;
        }
    }
}

int main(void)
{
    ServerContext ctx;
    int listen_fd = network_create_listening_socket();

    ctx.fds[0].fd     = listen_fd;
    ctx.fds[0].events = POLLIN;
    client_store_init(&ctx);

    run_event_loop(listen_fd, &ctx);

    close(listen_fd);
    return 0;
}