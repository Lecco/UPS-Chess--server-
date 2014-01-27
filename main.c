
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>

#include "constants.h"
#include "chess_game.h"
#include "communication.c"
#include "chess.c"

#define SERVER_LOG "chess_server.log"

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
        perror("Not enough parameters, run server as server <ip> <port>\nFor example ./ups 127.0.0.1 10000\n\n");
        exit(1);
    }
    char ip_address[strlen(argv[1])];
    strcpy(ip_address, argv[1]);
    int port = atoi(argv[2]);
    if (port < 1024 || port > 65535)
    {
        perror("Choose port number in range 1024-65535.");
        exit(6);
    }
    time_t mytime;
    int sock, connected_first, connected_second, true = 1;
    struct sockaddr_in first_player, second_player;
    int first_player_size, second_player_size;
    int ipv6 = 0;
    if (is_ipv6(ip_address))
    {
        ipv6 = 1;
        struct sockaddr_in6 server_addr;
        if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
        {
            perror("Couldn't create socket.");
            exit(2);
        }

        bzero((char *)&server_addr, sizeof(struct sockaddr_in6));
        server_addr.sin6_family = AF_INET6;
        server_addr.sin6_port = htons(port);
        server_addr.sin6_addr = in6addr_any;
        server_addr.sin6_scope_id = 2;
        inet_pton(AF_INET6, ip_address, (void *)&server_addr.sin6_addr.s6_addr);

        if (bind(sock, (struct sockaddr *)&server_addr, sizeof (server_addr)) == -1)
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
            char ipv6addr[256];
            printf("\nChess server waiting for client on ip address %s ", inet_ntop(AF_INET6, &server_addr.sin6_addr, ipv6addr, sizeof(ipv6addr)));
            printf("and port %d.\n", ntohs(server_addr.sin6_port));
            FILE *log = fopen(SERVER_LOG, "a");
            mytime = time(NULL);
            fprintf(log, "\n%sChess server waiting for client on ip address %s ", ctime(&mytime), inet_ntop(AF_INET6, &server_addr.sin6_addr, ipv6addr, sizeof(ipv6addr)));
            fprintf(log, "and port %d.\n", ntohs(server_addr.sin6_port));
            fclose(log);
        }
    }
    else
    {
        struct sockaddr_in server_addr;
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
            FILE *log = fopen(SERVER_LOG, "a");
            mytime = time(NULL);
            fprintf(log, "\n%sChess server waiting for client on ip address %s ", ctime(&mytime), inet_ntoa(server_addr.sin_addr));
            fprintf(log, "and port %d.\n", ntohs(server_addr.sin_port));
            fclose(log);
        }
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
        FILE *log = fopen(SERVER_LOG, "a");
        mytime = time(NULL);
        fprintf(log, "\n%sGAME %d: First player connected from %s, port %d\n", ctime(&mytime), number_of_game, inet_ntoa(first_player.sin_addr), ntohs(first_player.sin_port));
        fclose(log);
        
        // wait for next player, so game can begin
        second_player_size = sizeof (second_player);
        conn1 = 0;
        while (conn1 != 1)
        {
            connected_second = accept(sock, (struct sockaddr *) &second_player, &second_player_size);
            strcpy(conn1acc, receivePlayerData(connected_second));
            if (compare(conn1acc, succ) == 0)
            {
                conn1 = 1;
            }
            else
            {
                close(connected_second);
            }
        }
        sendConnectionInfo(connected_second, COMMAND_STATUS, COMMAND_SUCCESS);
        sendConnectionInfo(connected_second, COMMAND_COLOR, COMMAND_COLOR_BLACK);
        printf("\nGAME %d: Second player connected from %s, port %d\n", number_of_game, inet_ntoa(second_player.sin_addr), ntohs(second_player.sin_port));
        log = fopen(SERVER_LOG, "a"); 
        mytime = time(NULL);
        fprintf(log, "\n%sGAME %d: Second player connected from %s, port %d\n", ctime(&mytime), number_of_game, inet_ntoa(second_player.sin_addr), ntohs(second_player.sin_port));
        fclose(log);
        
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
                log = fopen(SERVER_LOG, "a");
                mytime = time(NULL);
                fprintf(log, "%sGAME %d: Turn of player %d\n", ctime(&mytime), number_of_game, game.player.reference);
                fclose(log);
                // sendPlayerCommand(game.player.reference, COMMAND_MESSAGE, "Your move:");
                int move_status = 0;
                char *move = (char *)malloc(sizeof(char) * 16);
                char *receivedMove = (char *) malloc(sizeof(char) * 16);
                while (move_status != 1 && game.white_player->connected == 1 && game.black_player->connected == 1)
                {
                    memset(move, '\0', sizeof(move));
                    memset(receivedMove, '\0', sizeof(receivedMove));
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
                log = fopen(SERVER_LOG, "a");
                mytime = time(NULL);
                fprintf(log, "%sGAME %d: Move %c%c%c%c\n", ctime(&mytime), number_of_game, receivedMove[0], receivedMove[1], receivedMove[2], receivedMove[3]);
                fclose(log);
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
                log = fopen(SERVER_LOG, "a");
                mytime = time(NULL);
                fprintf(log, "%sGAME %d: End of turn of player %d\n", ctime(&mytime), number_of_game, game.player.reference);
                fclose(log);
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

            FILE *log = fopen(SERVER_LOG, "a");
            mytime = time(NULL);
            printf("GAME %d: Ended", game.number);
            fprintf(log, "%sGAME %d: Ended", ctime(&mytime), game.number);
            if (game.check == WHITE_COLOR)
            {
                printf(", victorious is black player.\n");
                fprintf(log, ", victorious is black player.\n");
            }
            else if (game.check == BLACK_COLOR)
            {
                printf(", victorious is white player.\n");
                fprintf(log, ", victorious is white player.\n");
            }
            else
            {
                printf(", player disconnected.");
                fprintf(log, ", player disconnected.");
            }
            fclose(log);
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
