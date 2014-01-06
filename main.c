
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"



/**
 * Structure chess_game represents game of chess, meaning pieces on chess board,
 * their color.
 * 
 * @author Oldřich Pulkrt <O.Pulkrt@gmail.com>
 * @version 1.0
 */
struct chess_game
{
    /* In this array are position of figure (only figures without color) */
    int **board_figures;
    
    /* In this array are color of figure on fields */
    int **board_colors;
};

/**
 * Init chess board, fill it with info about pieces (their position and their
 * color).
 * 
 * @param game Game consisting of board (where pieces are and their color)
 */
void initChessBoard(struct chess_game *game)
{
    game->board_colors = (int **)malloc(sizeof(int *) * sizeof(int *) * CHESS_BOARD);
    game->board_figures = (int **)malloc(sizeof(int *) * sizeof(int *) * CHESS_BOARD);
    int i, j;
    for (i = 0; i < CHESS_BOARD; i++)
    {
        for (j = 0; j < CHESS_BOARD; j++)
        {
            game->board_colors[i][j] = DEFAULT_CHESSPIECE;
            game->board_figures[i][j] = DEFAULT_CHESSPIECE;
            if (i < WHITE_PLAYER_INIT)
            {
                game->board_colors[i][j] = WHITE_CHESSPIECE;
            }
            else if (i > BLACK_PLAYER_INIT)
            {
                game->board_colors[i][j] = BLACK_CHESSPIECE;
            }
        }
    }
    for (i = 0; i < CHESS_BOARD; i++)
    {
        game->board_figures[1][i] = PIECE_PAWN;
        game->board_figures[6][i] = PIECE_PAWN;
    }
    game->board_figures[0][0] = PIECE_ROOK;
    game->board_figures[0][1] = PIECE_KNIGHT;
    game->board_figures[0][2] = PIECE_BISHOP;
    game->board_figures[0][3] = PIECE_QUEEN;
    game->board_figures[0][4] = PIECE_KING;
    game->board_figures[0][5] = PIECE_BISHOP;
    game->board_figures[0][6] = PIECE_KNIGHT;
    game->board_figures[0][7] = PIECE_ROOK;
    
    game->board_figures[7][0] = PIECE_ROOK;
    game->board_figures[7][1] = PIECE_KNIGHT;
    game->board_figures[7][2] = PIECE_BISHOP;
    game->board_figures[7][3] = PIECE_QUEEN;
    game->board_figures[7][4] = PIECE_KING;
    game->board_figures[7][5] = PIECE_BISHOP;
    game->board_figures[7][6] = PIECE_KNIGHT;
    game->board_figures[7][7] = PIECE_ROOK;
}


/**
 * Structure player consists of player attributes like color of his pieces, if
 * he is victorious player (that is set after game is finished) and integer 
 * reference on his connection.
 * 
 * @author Oldřich Pulkrt <O.Pulkrt@gmail.com>
 * @version 1.0
 */
struct player
{
    /* Reference of connected player */
    int reference;
    
    /* Color of player */
    int color;
    
    /* True (1) if this user won */
    int victorious;
};


/**
 * Send message to player, of message couldn't be sent, this function will
 * print error message to stderr
 * 
 * @param connected Reference to connected player
 * @param command Command for player (for futher info read documentation)
 * @param param Param of command, differs by command
 */
void sendPlayerCommand(int connected, char *command, char *param)
{
    char replyBuffer[32];
    sprintf(replyBuffer, "%s\n", param);
    if (send(connected, replyBuffer, strlen(replyBuffer), 0) == -1)
    {
        perror("send() failed");
    }
}




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