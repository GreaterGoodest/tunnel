#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define KB 1024

#define CLIENT_ADDR "172.21.1.3"
#define CLIENT_PORT 1337
#define REMOTE_ADDR "172.21.0.3"
#define REMOTE_PORT 1337
#define MAX_TCP 1 * KB

/**
 * @brief Sets up the client socket
 * 
 * @param client_sock pointer to socket fd to setup
 * @return int error code
 */
int setup_client_conn(int *client_sock)
{
    int status = 0;
    struct sockaddr_in client_addr;

    *client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*client_sock == -1)
    {
        perror("setup_client_sock: socket");
        return -1;
    }

    client_addr.sin_family = AF_INET;
    status = inet_pton(AF_INET, CLIENT_ADDR, &(client_addr.sin_addr));
    if (status != 1)
    {
        perror("setup_client_sock: inet_pton");
        return -1;
    }
    client_addr.sin_port = htons(CLIENT_PORT);

    status = connect(*client_sock, (struct sockaddr *)&client_addr, sizeof(client_addr));
    if (status == -1)
    {
        perror("setup_client_sock: connect");
        return status;
    }
    status = fcntl(*client_sock, F_SETFL, fcntl(*client_sock, F_GETFL, 0) | O_NONBLOCK);
    if (status == -1)
    {
        perror("setup_client_sock fnctl");
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


int main(int argc, char **argv)
{
    int status = 0;
    int client_sock = 0;  // Receives proxy requests
    int remote_sock = 0;  // Proxy target
    struct sockaddr_in conn_addr = {0};

    status = setup_client_conn(&client_sock);
    if (status < 0)
    {
        puts("main: Failed to setup local listener.");
        return 1;
    }

    while(1){ 
            if (!remote_sock){
                status = setup_remote_sock(&remote_sock);
                if (status != 0)
                {
                    puts("main: Failed to setup remote socket.");
                    return 1;
                }
            }
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