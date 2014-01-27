
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "constants.h"
#include "chess_game.h"
#include "communication.c"
#include "chess.c"

/**
 * Main chess server function. 
 * 
 * @param argc Count of arguments
 * @param argv Arguments
 * @return Any integer different from zero means error
 */
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Not enough parameters, run server as server <ip> <port>");
        exit(1);
    }
    char ip_address[strlen(argv[1])];
    strcpy(ip_address, argv[1]);
    int port = atoi(argv[2]);
    int sock, connected_first, connected_second, true = 1;

    struct sockaddr_in server_addr, first_player, second_player;
    int first_player_size, second_player_size;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Couldn't create socket.");
        exit(2);
    }

    if (argv[1][0] >= (char)'A' && argv[1][0] <= (char)'z')
    {
        hostname_to_ip(ip_address);
    }
    
    if (is_valid_ip(ip_address) == 0)
    {
        perror("Not valid IP address.\n");
        exit(5);
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

    socklen_t len = sizeof(server_addr);
    if (getsockname(sock, (struct sockaddr *) &server_addr, &len))
    {
        perror("Can't get socket name.\n");
    }
    else
    {
        printf("\nChess server waiting for client on ip address %s ", inet_ntoa(server_addr.sin_addr));
        printf("and port %d.\n", ntohs(server_addr.sin_port));
    }
    printf("\n");
    int number_of_game = 0;

    while (1)
    {
        int conn1;
        char conn1acc[1024];
        char succ[1024];
        strcpy(succ, "STATUS---SUCCESS");
        conn1 = 0;
        number_of_game++;

        first_player_size = sizeof (first_player);
        while (conn1 != 1)
        {
            connected_first = accept(sock, (struct sockaddr *) &first_player, &first_player_size);
            strcpy(conn1acc, receivePlayerData(connected_first));
            if (compare(conn1acc, succ) == 0)
            {
                conn1 = 1;
            }
            else
            {
                close(connected_first);
            }
        }
        sendConnectionInfo(connected_first, COMMAND_STATUS, COMMAND_SUCCESS);
        sendConnectionInfo(connected_first, COMMAND_COLOR, COMMAND_COLOR_WHITE);
        printf("\nGAME %d: First player connected from %s, port %d\n", number_of_game, inet_ntoa(first_player.sin_addr), ntohs(first_player.sin_port));
        
        // wait for next player, so game can begin
        second_player_size = sizeof (second_player);
        conn1 = 0;
        while (conn1 != 1)
        {
            connected_second = accept(sock, (struct sockaddr *) &second_player, &second_player_size);
            strcpy(conn1acc, receivePlayerData(connected_second));
            if (compare(conn1acc, "STATUS---SUCCESS"))
            {
                conn1 = 1;
            }
        }
        sendConnectionInfo(connected_second, COMMAND_STATUS, COMMAND_SUCCESS);
        sendConnectionInfo(connected_second, COMMAND_COLOR, COMMAND_COLOR_BLACK);
        printf("\nGAME %d: Second player connected from %s, port %d\n", number_of_game, inet_ntoa(second_player.sin_addr), ntohs(second_player.sin_port));
        
        sendConnectionInfo(connected_first, COMMAND_BLACK_STATUS, COMMAND_CONNECTED);
        sendConnectionInfo(connected_second, COMMAND_WHITE_STATUS, COMMAND_CONNECTED);

        int fork_id = fork();
        
        if (fork_id == 0)
        {
            // set white and black player
            printf("GAME %d: Setting white and black player\n", number_of_game);
            struct player white_player, black_player;
            struct chess_game game;
            game.number = number_of_game;
            game.state = STATUS_DEFAULT;
            game.check = DEFAULT_COLOR;
            
            white_player.color = WHITE_COLOR;
            white_player.reference = connected_first;
            white_player.connected = 1;
            black_player.color = BLACK_COLOR;
            black_player.reference = connected_second;
            black_player.connected = 1;
            game.white_player = &white_player;
            game.black_player = &black_player;

            // init chessboard
            printf("GAME %d: Initialization of chessboard\n", number_of_game);
            initChessBoard(&game);
            
            printf("GAME %d: Game begins\n", number_of_game);
            int check_mate = 0, check_stalemate = 0;
            game.player = white_player;
            while (!check_mate && !check_stalemate && game.white_player->connected == 1 && game.black_player->connected == 1)
            {
                // game loop
                printf("GAME %d: Turn of player %d\n", number_of_game, game.player.reference);
                // sendPlayerCommand(game.player.reference, COMMAND_MESSAGE, "Your move:");
                int move_status = 0;
                char *move = (char *)malloc(sizeof(char) * 16);
                char *receivedMove = (char *) malloc(sizeof(char) * 16);
                while (move_status != 1 && game.white_player->connected == 1 && game.black_player->connected == 1)
                {
                    move = receivePlayerData(game.player.reference);
                    if (((move[0] >= 'a' && move[0] <= 'h') || (move[0] >= 'A' && move[0] <= 'H')) &&
                        (move[1] >= '1' && move[1] <= '8') &&
                        ((move[2] >= 'a' && move[2] <= 'h') || (move[2] >= 'A' && move[2] <= 'H')) &&
                        (move[3] >= '1' && move[3] <= '8'))
                    {
                        strcpy(receivedMove,  move);
                        move_status = playMove(&game, move);
                    }
                    else
                    {
                        move_status = 0;
                    }
                }
                if (game.player.reference == white_player.reference)
                {
                    sendPlayerCommand(black_player.reference, COMMAND_MOVE, receivedMove, &game);
                }
                else
                {
                    sendPlayerCommand(white_player.reference, COMMAND_MOVE, receivedMove, &game);
                }
                if (game.check == WHITE_COLOR)
                {
                    printf("White player is checked\n");
                    if (isCheckmate(&game))
                    {
                        check_mate = 1;
                    }
                }
                if (game.check == BLACK_COLOR)
                {
                    printf("Black player is checked\n");
                    if (isCheckmate(&game))
                    {
                        check_mate = 1;
                    }
                }
                printf("GAME %d: End of turn of player %d\n", number_of_game, game.player.reference);
                if (check_mate)
                {
                    sendPlayerCommand(white_player.reference, COMMAND_GAME_STATUS, COMMAND_GAME_STATUS_CHECKMATE, &game);
                    sendPlayerCommand(black_player.reference, COMMAND_GAME_STATUS, COMMAND_GAME_STATUS_CHECKMATE, &game);
                    // send players info about victorious player
                    if (game.check == WHITE_COLOR)
                    {
                        sendPlayerCommand(white_player.reference, COMMAND_WHITE_PLAYER, COMMAND_GAME_STATUS_CHECKMATE, &game);
                        sendPlayerCommand(black_player.reference, COMMAND_WHITE_PLAYER, COMMAND_GAME_STATUS_CHECKMATE, &game);
                    }
                    else
                    {
                        sendPlayerCommand(white_player.reference, COMMAND_BLACK_PLAYER, COMMAND_GAME_STATUS_CHECKMATE, &game);
                        sendPlayerCommand(black_player.reference, COMMAND_BLACK_PLAYER, COMMAND_GAME_STATUS_CHECKMATE, &game);
                    }
                }
                else if (check_stalemate)
                {
                    sendPlayerCommand(white_player.reference, COMMAND_GAME_STATUS, COMMAND_GAME_STATUS_STALEMATE, &game);
                    sendPlayerCommand(black_player.reference, COMMAND_GAME_STATUS, COMMAND_GAME_STATUS_STALEMATE, &game);
                }
                else if (game.check != DEFAULT_COLOR)
                {
                    sendPlayerCommand(white_player.reference, COMMAND_GAME_STATUS, COMMAND_GAME_STATUS_CHECK, &game);
                    sendPlayerCommand(black_player.reference, COMMAND_GAME_STATUS, COMMAND_GAME_STATUS_CHECK, &game);
                }
                else
                {
                    sendPlayerCommand(white_player.reference, COMMAND_GAME_STATUS, COMMAND_GAME_STATUS_DEFAULT, &game);
                    sendPlayerCommand(black_player.reference, COMMAND_GAME_STATUS, COMMAND_GAME_STATUS_DEFAULT, &game);
                }
                // send players statuses of their opponents
                int whitePlayerConnected = sendPlayerCommand(white_player.reference, COMMAND_MESSAGE, "Checking if player is connected.", &game);
                int blackPlayerConnected = sendPlayerCommand(black_player.reference, COMMAND_MESSAGE, "Checking if player is connected.", &game);
                if (whitePlayerConnected)
                {
                    sendPlayerCommand(black_player.reference, COMMAND_WHITE_STATUS, COMMAND_CONNECTED, &game);
                }
                else
                {
                    sendPlayerCommand(black_player.reference, COMMAND_WHITE_STATUS, COMMAND_DISCONNECTED, &game);
                }
                if (blackPlayerConnected)
                {
                    sendPlayerCommand(white_player.reference, COMMAND_BLACK_STATUS, COMMAND_CONNECTED, &game);
                }
                else
                {
                    sendPlayerCommand(white_player.reference, COMMAND_BLACK_STATUS, COMMAND_DISCONNECTED, &game);
                }
                if (game.player.reference == white_player.reference)
                {
                    game.player = black_player;
                }
                else
                {
                    game.player = white_player;
                }
            }

            printf("GAME %d: Ended", game.number);
            if (game.check == WHITE_COLOR)
            {
                printf(", victorious is black player.\n");
            }
            else if (game.check == BLACK_COLOR)
            {
                printf(", victorious is white player.\n");
            }
            else
            {
                printf(", player disconnected.");
            }
            close(connected_first);
            close(connected_second);
            break;
        }
        else
        {
            close(connected_first);
            close(connected_second);
        }
    }
}
