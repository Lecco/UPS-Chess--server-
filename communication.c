/* 
 * This file contains functions for communicating between client and server
 * 
 * File:   communication.c
 * Author: Oldřich Pulkrt <O.Pulkrt@gmail.com>
 *
 * Created on 25. leden 2014, 10:51
 */

/**
 * Send message to player, of message couldn't be sent, this function will
 * print error message to stderr
 * 
 * @param connected Reference to connected player
 * @param command Command for player (for futher info read documentation)
 * @param param Param of command, differs by command
 * @return True if sending was ok
 */
int sendPlayerCommand(int connected, char *command, char *param, struct chess_game *game)
{
    if ((game->white_player->reference == connected && game->white_player->connected == 0) ||
        (game->black_player->reference == connected && game->black_player->connected == 0) ||
        connected == -1)
    {
        return 0;
    }
    char replyBuffer[1024];
    sprintf(replyBuffer, "%s---%s\n", command, param);
    if (send(connected, replyBuffer, strlen(replyBuffer), 0) == -1)
    {
        perror("send() failed");
        if (game->white_player->reference == connected)
        {
            game->white_player->connected = 0;
            sendConnectionInfo(game->black_player->reference, COMMAND_WHITE_STATUS, COMMAND_DISCONNECTED);
        }
        else
        {
            game->black_player->connected = 0;
            sendConnectionInfo(game->white_player->reference, COMMAND_BLACK_STATUS, COMMAND_DISCONNECTED);
        }
        return 0;
    }
    return 1;
}

/**
 * Send info about connection
 *
 * @param connected Reference to player
 * @param command status command
 * @param Param of command
 */
int sendConnectionInfo(int connected, char *command, char *param)
{
    char replyBuffer[1024];
    sprintf(replyBuffer, "%s---%s\n", command, param);
    if (send(connected, replyBuffer, strlen(replyBuffer), 0) == -1)
    {
        perror("send() failed");
        return 0;
    }
    return 1;
}

/**
 * Receive data from client
 * 
 * @param connected Identifier of player
 * @return Data from client
 */
char* receivePlayerData(int connected)
{
    int bytes_received;
    char recv_data;
    char *data = (char *)malloc(sizeof(char) * 1024);
    int i;
    for (i = 0; (bytes_received = recv(connected, &recv_data, 1, 0)); i++)
    {
        if (recv_data == '\n')
        {
            break;
        }
        data[i] = recv_data;
    }
    return data;
}

int hostname_to_ip(char * ip)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ip, "http", &hints, &servinfo)) != 0)
    {
        perror("getaddrinfo");
        return 1;
    }

    for (p = servinfo; p!= NULL; p = p->ai_next)
    {
        h = ((struct sockaddr_in *)p->ai_addr);
        strcpy(ip, inet_ntoa(h->sin_addr));
        printf("%s\n", ip);
        //break;
    }

    freeaddrinfo(servinfo);
    return 0;
}

int is_valid_ip(char *ip)
{
    // check if ip4
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
    if (result == 1)
    {
        return 1;
    }

    return is_ipv6(ip);
}

int is_ipv6(char *ip)
{
    struct addrinfo hints, *servinfo;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    rv = getaddrinfo(ip, NULL, &hints, &servinfo);

    if (servinfo->ai_family == AF_INET6)
    {
        return 1;
    }
    return 0;
}

int compare(char a[], char b[])
{
    int c = 0;

    while(a[c] == b[c])
    {
        if(a[c] == '\0' || b[c] == '\0')
            break;
        c++;

    }
    if(a[c] == '\0' || b[c] == '\0')
        return 0;
    else
        return -1;
}
