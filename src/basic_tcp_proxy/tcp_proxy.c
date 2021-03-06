#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define KB 1024

#define LISTEN_ADDR "127.0.0.1"
#define LISTEN_PORT 1337
#define REMOTE_ADDR "127.0.0.1"
#define REMOTE_PORT 1338
#define MAX_TCP 1 * KB

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

    //Create listening socket to receive proxy requests
    *listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*listen_sock == -1)
    {
        perror("setup_local_listener socket");
        return status;
    }
    status = setsockopt(*listen_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if (status == -1)
    {
        perror("setup_local_listener setsockopt");
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

/**
 * @brief Set up the remote socket
 * 
 * @param remote_sock target socket to proxy traffic to
 * @return int error code
 */
int setup_remote_sock(int *remote_sock)
{
    int status = 0;
    struct sockaddr_in remote_addr;

    *remote_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*remote_sock == -1)
    {
        perror("setup_remote_sock: socket");
        return -1;
    }

    remote_addr.sin_family = AF_INET;
    status = inet_pton(AF_INET, REMOTE_ADDR, &(remote_addr.sin_addr));
    if (status != 1)
    {
        perror("setup_remote_sock: inet_pton");
        return -1;
    }
    remote_addr.sin_port = htons(REMOTE_PORT);

    status = connect(*remote_sock, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
    if (status == -1)
    {
        perror("setup_remote_sock: connect");
        return status;
    }
    status = fcntl(*remote_sock, F_SETFL, fcntl(*remote_sock, F_GETFL, 0) | O_NONBLOCK);
    if (status == -1)
    {
        perror("setup_local_listener fnctl");
        return status;
    }

    return status;
}

/**
 * @brief Checks for new data from client/server
 * 
 * @param client_sock connected client socket
 * @param remote_sock socket connection to target host
 * @return int error code
 */
int data_checks(int client_sock, int remote_sock)
{
    int status = 0;

    char data[MAX_TCP] = {0};
    status = read(client_sock, data, sizeof(data)-1);
    if (strlen(data) > 0)
    {
        write(remote_sock, data, strlen(data)+1);
        memset(data, 0, sizeof(data));
    }
    status = read(remote_sock, data, sizeof(data)-1);
    if (strlen(data) > 0)
    {
        write(client_sock, data, strlen(data)+1);
        memset(data, 0, sizeof(data));
    }

    return status;
}


int main()
{
    int status = 0;
    int listen_sock = 0;  // Receives proxy requests
    int remote_sock = 0;  // Proxy target
    int client_sock = 0;  // Client to proxy
    struct sockaddr_in client_addr = {0};

    status = setup_local_listener(&listen_sock);
    if (status < 0)
    {
        puts("main: Failed to setup local listener.");
        return 1;
    }

    client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &(int){0});
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

    status = setup_remote_sock(&remote_sock);
    if (status != 0)
    {
        puts("main: Failed to setup remote socket.");
        return 1;
    }

    while(1){ 
        status = data_checks(client_sock, remote_sock);
        if (status < 0 && errno != EAGAIN)
        {
            puts("main: Failed data checks.");
            return 1;
        }else if (status == 0){
            puts("connection closed");
            close(client_sock);
            close(remote_sock);
            client_sock = 0;
            remote_sock = 0;
        }
    }

    return 0;
}