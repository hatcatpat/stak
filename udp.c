#include "stak.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 2000
#define MAX_MESSAGE_LENGTH 2000

bool server_running = false;

enum status
server_callback(char *str, size_t len)
{
    lexer(str, len);
    return OK;
}

enum status
run_server()
{
    int socket_desc;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    char message[MAX_MESSAGE_LENGTH];
    socklen_t client_struct_length = sizeof(client_addr);

    memset(message, '\0', sizeof(message));

    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (socket_desc < 0)
        {
            printf("[SERVER] socket failed to create\n");
            return BAD;
        }
    printf("[SERVER] socket created...\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            printf("[SERVER] binding failed!\n");
            close(socket_desc);
            return BAD;
        }
    printf("[SERVER] binding succeeded...\n");

    printf("[SERVER] server running on 127.0.0.1:%i...\n", SERVER_PORT);

    server_running = true;
    while (server_running)
        {
            if (recvfrom(socket_desc, message, sizeof(message), 0, (struct sockaddr *)&client_addr, &client_struct_length) < 0)
                {
                    printf("[SERVER] failed to receive!\n");
                    close(socket_desc);
                    return BAD;
                }

            printf("[SERVER] received message from ip: %s and port: %i\n%s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message);

            if(strlen(message) == 1 || server_callback(message, strlen(message)))
                break;

            memset(message, 0, sizeof(message));
        }
    server_running = false;

    close(socket_desc);
    return OK;
}
