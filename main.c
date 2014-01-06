#include <stdio.h>   /* standard in and output*/
#include <sys/socket.h> /* for socket() and socket functions*/
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>    
#include <string.h>     
#include <unistd.h>     /* for close() */

#include "constants.h"
#include "Chess_game.c"

int main(int argc, char *argv[])
{
    int port = 10001;
    char *ip_address = "127.0.0.1";
    int sock, connected_first, connected_second, bytes_received, true = 1;
    char recv_data;
    char replyBuffer[32];

    struct sockaddr_in server_addr, first_player, second_player;
    int first_player_size, second_player_size;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Couldn't create socket.");
        exit(1);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof (int)) == -1)
    {
        perror("Setsockopt");
        exit(2);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    bzero(&(server_addr.sin_zero), 8);

    if (bind(sock, (struct sockaddr *) &server_addr, sizeof (server_addr)) == -1)
    {
        perror("Unable to bind.");
        exit(3);
    }

    if (listen(sock, 5) == -1)
    {
        perror("Unable to listen.");
        exit(4);
    }

    printf("\nChess server waiting for client on port %d..", port);

    while (1)
    {
        first_player_size = sizeof (first_player);
        connected_first = accept(sock, (struct sockaddr *) &first_player, &first_player_size);
        printf("\nFirst player connected from %s on port %d)", inet_ntoa(first_player.sin_addr), ntohs(first_player.sin_port));
        
        // wait for next player, so game can begin
        second_player_size = sizeof (second_player);
        connected_second = accept(sock, (struct sockaddr *) &second_player, &second_player_size);
        printf("\nSecond player connected from %s on port %d)", inet_ntoa(second_player.sin_addr), ntohs(second_player.sin_port));

        if (fork() == 0)
        {
            while ((bytes_received = recv(connected_first, &recv_data, 1, 0)))
            {
                printf("\nrecv= %c\n", recv_data);
                if (recv_data == '\n')
                {
                    break;
                }
            }
            
            int success = 1;
            sprintf(replyBuffer, "%d\n", success);
            //printf("reply buffer = %s\n", replyBuffer);
            if (send(connected_first, replyBuffer, strlen(replyBuffer), 0) == -1)
            {
                perror("send() failed");
            }
            success = 0;
            
        }
        else
        {
            close(connected_first);
            close(connected_second);
        }
    }
}