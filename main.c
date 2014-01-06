#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"
#include "chess_game.c"
#include "player.c"

/**
 * Main chess server function. 
 * 
 * @param argc Count of arguments
 * @param argv Arguments
 * @return Any integer different from zero means error
 */
int main(int argc, char *argv[])
{
    int port = 10001;
    char *ip_address = "127.0.0.1";
    int sock, connected_first, connected_second, bytes_received, true = 1;
    char recv_data;

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
    
    int number_of_game = 0;

    while (1)
    {
        first_player_size = sizeof (first_player);
        connected_first = accept(sock, (struct sockaddr *) &first_player, &first_player_size);
        printf("\nFirst player connected from %s on port %d)\n", inet_ntoa(first_player.sin_addr), ntohs(first_player.sin_port));
        
        // wait for next player, so game can begin
        second_player_size = sizeof (second_player);
        connected_second = accept(sock, (struct sockaddr *) &second_player, &second_player_size);
        printf("\nSecond player connected from %s on port %d)\n", inet_ntoa(second_player.sin_addr), ntohs(second_player.sin_port));
        
        number_of_game++;
        
        sendPlayerCommand(connected_first, COMMAND_STATUS, COMMAND_SUCCESS);
        sendPlayerCommand(connected_second, COMMAND_STATUS, COMMAND_SUCCESS);
        
        printf("\n\n\n");

        if (fork() == 0)
        {
            // set white and black player
            printf("Setting white and black player\n");
            struct player white_player, black_player;
            struct chess_game game;
            
            white_player.color = WHITE_PLAYER;
            white_player.reference = connected_first;
            white_player.victorious = 0;
            black_player.color = BLACK_PLAYER;
            black_player.reference = connected_second;
            black_player.victorious = 0;
            
            // init chessboard
            printf("Initialization of chessboard\n");
            initChessBoard(&game);
            
            printf("Game begins\n");
            int check_mate = 0, check_stalemate = 0;
            while (!check_mate && !check_stalemate)
            {
                // game loop
            }
            
            
            while ((bytes_received = recv(connected_first, &recv_data, 1, 0)))
            {
                printf("\nrecv= %c\n", recv_data);
                if (recv_data == '\n')
                {
                    break;
                }
            }
        }
        else
        {
            close(connected_first);
            close(connected_second);
        }
        sleep(5000);
    }
}