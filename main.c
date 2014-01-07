
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"



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
    
    /* Identifier of user, whose turn is now */
    struct player player;
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
        game->board_colors[i] = (int *)malloc(sizeof(int *) * CHESS_BOARD);
        game->board_figures[i] = (int *)malloc(sizeof(int *) * CHESS_BOARD);
        for (j = 0; j < CHESS_BOARD; j++)
        {
            game->board_colors[i][j] = DEFAULT_COLOR;
            game->board_figures[i][j] = DEFAULT_COLOR;
            if (i < WHITE_PLAYER_INIT)
            {
                game->board_colors[i][j] = WHITE_COLOR;
            }
            else if (i > BLACK_PLAYER_INIT)
            {
                game->board_colors[i][j] = BLACK_COLOR;
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
 * Prints chess board (without info about color)
 * 
 * @param game Game consists of info about positions of pieces and their color
 */
void printChessBoard(struct chess_game *game)
{
    int i, j;
    for (i = CHESS_BOARD - 1; i >= 0; i--)
    {
        for (j = 0; j < CHESS_BOARD; j++)
        {
            printf("%d ", game->board_figures[i][j]);
        }
        printf("\n");
    }
}

/**
 * Prints colors of pieces on chess board
 * 
 * @param game Game consists of info about colors
 */
void printChessBoardColors(struct chess_game *game)
{
    int i, j;
    for (i = CHESS_BOARD - 1; i >= 0; i--)
    {
        for (j = 0; j < CHESS_BOARD; j++)
        {
            printf("%d ", game->board_colors[i][j]);
        }
        printf("\n");
    }
}

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
    printf("data = %s\n", data);
    return data;
}

/**
 * According to returned integer code we can find out, if this move is possible
 * to play (it is chess, so piece can not end up outside chessboard or user can't
 * move to position, where he'll have check/checkmate).
 * 
 * @param game Game with info about pieces
 * @param move Move we want to sort out
 * @return Code (check documentation for more info)
 */
int playMove(struct chess_game *game, char *move)
{
    printf("Trying to play move %s\n", move);
    int movePlayable = isMovePlayable(game, move);
    if (movePlayable == MOVE_PLAYABLE)
    {
        printf("Move is playable\n");
        // play this move
    }
    else
    {
        printf("Move can not be played, reason = %d\n", movePlayable);
        // print reason
    }
    sleep(1000);
}

/**
 * Check if move is playable in current distribution of pieces
 * 
 * @param game Info about pieces
 * @param move Move to check
 * @return True if move can be played
 */
int isMovePlayable(struct chess_game *game, char *move)
{
    int i;
    for (i = 0; i < MOVE_LENGTH; i++)
    {
        // correct move to indexes of array
        if (move[i] >= ASCII_CAPITAL_A && move[i] <= ASCII_CAPITAL_H)
        {
            move[i] = move[i] - ASCII_CAPITAL_A;
        }
        if (move[i] >= ASCII_A && move[i] <= ASCII_H)
        {
            move[i] = move[i] - ASCII_A;
        }
        if (move[i] >= ASCII_0 && move[i] <= ASCII_9)
        {
            move[i] = move[i] - ASCII_0 - 1;
        }
        // check if move starts and ends on chess board
        if (move[i] < 0 || move[i] >= CHESS_BOARD)
        {
            return MOVE_NOT_CHESSBOARD;
        }
    }
    printChessBoardColors(game);
    if (game->board_colors[move[1]][move[0]] == game->board_colors[move[3]][move[2]])
    {
        return MOVE_OWN_PIECE;
    }
    printf("player = %d, piece = %d\n", game->player.color, game->board_colors[move[1]][move[0]]);
    if (game->player.color != game->board_colors[move[1]][move[0]])
    {
        return MOVE_NOT_OWNER;
    }
    return MOVE_PLAYABLE;
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
    int sock, connected_first, connected_second, true = 1;
    

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
        printf("\nFirst player connected from %s, port %d\n", inet_ntoa(first_player.sin_addr), ntohs(first_player.sin_port));
        
        // wait for next player, so game can begin
        second_player_size = sizeof (second_player);
        connected_second = accept(sock, (struct sockaddr *) &second_player, &second_player_size);
        printf("\nSecond player connected from %s, port %d\n", inet_ntoa(second_player.sin_addr), ntohs(second_player.sin_port));
        
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
            
            white_player.color = WHITE_COLOR;
            white_player.reference = connected_first;
            white_player.victorious = 0;
            black_player.color = BLACK_COLOR;
            black_player.reference = connected_second;
            black_player.victorious = 0;
            
            // init chessboard
            printf("Initialization of chessboard\n");
            initChessBoard(&game);
            
            printf("Game begins\n");
            int check_mate = 0, check_stalemate = 0;
            game.player = white_player;
            while (!check_mate && !check_stalemate)
            {
                // game loop
                printf("Turn of player %d\n", game.player.reference);
                sendPlayerCommand(game.player.reference, COMMAND_MESSAGE, "Your move:");
                char *move;
                move = receivePlayerData(game.player.reference);
                int moveStatus = playMove(&game, move);
                printf("End of turn of player %d", game.player.reference);
                if (game.player.reference == white_player.reference)
                {
                    game.player = black_player;
                }
                else
                {
                    game.player = white_player;
                }
            }
        }
        else
        {
            close(connected_first);
            close(connected_second);
        }
    }
}