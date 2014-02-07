/* 
 * This file contains functions for communicating between client and server
 * 
 * File:   communication.c
 * Author: Oldřich Pulkrt <O.Pulkrt@gmail.com>
 *
 * Created on 25. leden 2014, 10:51
 */

#ifndef COMMUNICATION_C
#define COMMUNICATION_C

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "constants.h"
#include "chess_game.h"


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

/**
 * Return ip address by DNS
 * 
 * @param ip hostname
 * @return IP address
 */
int hostname_to_ip(char * ip)
{

/*
struct sockaddr SA;
    static char HOST[25];
// pretend sa is full of good information about the host and port...

	if (getnameinfo(&SA, sizeof(SA), HOST, sizeof(HOST), NULL, NULL, 0))
	{
		printf("ne");
		return 0;
	}
printf("a");
return 1;
*/
/*

        struct hostent *he;
	struct in_addr **addr_list;
	int i;
		
	if ( (he = gethostbyname( ip ) ) == NULL) 
	{
		// get the host info
		herror("gethostbyname");
		return 1;
	}

	addr_list = (struct in_addr **) he->h_addr_list;
	
	for(i = 0; addr_list[i] != NULL; i++) 
	{
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		return 0;
	}
	
	return 1;
*/

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_in *h;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ip, NULL, &hints, &servinfo)) != 0)
    {
        perror("getaddrinfo");
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        h = ((struct sockaddr_in *)p->ai_addr);
        strcpy(ip, inet_ntoa(h->sin_addr));
        break;
    }

    freeaddrinfo(servinfo);
    return 0;

}

/**
 * Check if ip address is valid (ipv4 or ipv6)
 * 
 * @param ip IP address
 * @return True if ip is valid IP address
 */
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

/**
 * Check if IP address is valid ipv6
 * 
 * @param ip IP address
 * @return True if ip is valid ipv6 addres
 */
int is_ipv6(char *ip, int port)
{
    struct addrinfo hints, *servinfo;
    int rv;

    char portik[16];
    snprintf(portik, sizeof(portik), "%ld", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    rv = getaddrinfo(ip, portik, &hints, &servinfo);
    if (rv < 0)
    {
        return 0;
    }

    if (rv < 0 ||servinfo->ai_family == AF_INET6)
    {
        return 1;
    }
    return 0;
}

/**
 * Compare two strings
 * 
 * @param a First string
 * @param b Second string
 * @return True if string are same
 */
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


#endif
