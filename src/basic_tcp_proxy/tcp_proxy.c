#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LISTEN_ADDR "127.0.0.1"
#define LISTEN_PORT 1337

/**
 * @brief Sets up the local listener socket
 * 
 * @param listen_sock pointer to socket fd to setup
 * @return int error code
 */
int setup_local_listener(int *listen_sock)
{
    int status = 0;
    struct sockaddr_in listen_addr = {0};

    listen_addr.sin_family = AF_INET;
    status = inet_pton(AF_INET, LISTEN_ADDR, &(listen_addr.sin_addr));
    if (status != 1)
    {
        perror("setup_local_listener inet_pton");
        return -1;
    }
    listen_addr.sin_port = htons(LISTEN_PORT);

    //Create non-blocking listening socket to receive proxy requrests
    *listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (status == -1)
    {
        perror("setup_local_listener socket");
        return status;
    }
    status = fcntl(*listen_sock, F_SETFL, fcntl(*listen_sock, F_GETFL, 0) | O_NONBLOCK);
    if (status == -1)
    {
        perror("setup_local_listener fnctl");
        return status;
    }
    status = bind(*listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr));
    if (status == -1)
    {
        perror("setup_local_listener bind");
        return status;
    }
    status = listen(*listen_sock, SOMAXCONN);
    if (status == -1)
    {
        perror("setup_local_listener listen");
        return status;
    }

    return status;
}

int main(int argc, char **argv)
{
    int status = 0;
    int listen_sock = 0;  // Receives proxy requests
    int remote_sock = 0;  // Proxy target
    int client_sock = 0;  // Client to proxy
    int client_addr_len = 0;
    struct sockaddr_in client_addr = {0};

    status = setup_local_listener(&listen_sock);
    if (status < 0)
    {
        puts("Failed to setup local listener.");
        return 1;
    }

    while(1){ 
        if (client_sock <= 0)
        {
            client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_sock == -1)
            {
                if (errno != EAGAIN){
                    perror("main accept");
                    return errno;
                }
            } else 
            {
                puts("accepted");
                status = fcntl(client_sock, F_SETFL, fcntl(client_sock, F_GETFL, 0) | O_NONBLOCK);
                if (status == -1)
                {
                    perror("main client_sock fnctl");
                    return status;
                }
            }
        }else {
            char data[256] = {0};
            read(client_sock, data, sizeof(data)-1);
            if (strlen(data) > 0)
            {
                printf("%s", data);
            }
        }
    }

    return 0;
}