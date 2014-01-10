
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

    /* Identifier of game */
    int number;
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
    char replyBuffer[1024];
    sprintf(replyBuffer, "%s---%s\n", command, param);
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
    printf("GAME %d: Trying to play move %s\n", game->number, move);
    int movePlayable = isMovePlayable(game, move);
    switch (movePlayable)
    {
        case MOVE_NOT_CHESSBOARD:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "Piece would end outside chessboard, try again.");
            break;
        case MOVE_NOT_OWNER:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "You tried to move other players piece.");
            break;
        case MOVE_OWN_PIECE:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "You tried to move on field with yout piece on it, try again.");
            break;
        case MOVE_NOT_PLAYABLE:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "Piece can not perform this move.");
            break;
        case MOVE_PLAYABLE:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_SUCCESS);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "Move successfully completed.");
            break;
        default:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "Incorrect move.");
            break;
    }
    return movePlayable == MOVE_PLAYABLE ? 1 : 0;
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
    // check if ending field isn't already used by players piece
    if (game->board_colors[move[1]][move[0]] == game->board_colors[move[3]][move[2]])
    {
        return MOVE_OWN_PIECE;
    }
    // check if player is moving with his own piece
    if (game->player.color != game->board_colors[move[1]][move[0]])
    {
        return MOVE_NOT_OWNER;
    }
    // check if move can be performed by this piece
    if (pieceMove(game, move) != 1)
    {
        return MOVE_NOT_PLAYABLE;
    }
    return MOVE_PLAYABLE;
}

/**
 * Check if piece can move this way
 * 
 * @param game Chessboard
 * @param move Move of chesspiece
 * @return True if chesspiece that move is possible
 */
int pieceMove(struct chess_game *game, char *move)
{
    int piece = game->board_figures[move[1]][move[0]];
    switch (piece)
    {
        case PIECE_BISHOP:
            if (move[1] - move[0] != move[3] - abs(move[2]))
            {
                return 0;
            }
            // check if there isn't any other piece in the way
            // TODO
            break;
        case PIECE_KING:
            if (abs(move[0] - move[2]) > 1 || abs(move[1] - move[3]) > 1)
            {
                return 0;
            }
            break;
        case PIECE_KNIGHT:
            if (!((abs(move[3] - move[1]) == 2 && abs(move[0] - move[2]) == 1) || 
                (abs(move[3] - move[1]) == 1 && abs(move[0] - move[2]) == 2)))
            {
                return 0;
            }
            break;
        case PIECE_PAWN:
            // TODO: capturing other players piece (diagonal)
            
            if (!(abs(move[3] - move[1]) == 1 ||
                (move[3] - move[1] == 2 && game->player.color == WHITE_COLOR) ||
                (move[1] - move[3] == 2 && game->player.color == BLACK_COLOR)))
            {
                return 0;
            }
            break;
        case PIECE_QUEEN:
            if (move[1] - move[0] != move[3] - abs(move[2]) ||
                !(abs(move[2] - move[0]) > 0 && abs(move[3] - move[1]) == 0) ||
                !(abs(move[3] - move[1]) > 0 && abs(move[2] - move[1]) == 0))
            {
                return 0;
            }
            break;
        case PIECE_ROOK:
            if (!(abs(move[2] - move[0]) > 0 && abs(move[3] - move[1]) == 0) ||
                !(abs(move[3] - move[1]) > 0 && abs(move[2] - move[1]) == 0))
            {
                return 0;
            }
            break;
    }
    return 1;
}


/**
 * Testing function
 * 
 * @param piece Chess piece
 * @param move Move we want to perform
 * @return True if piece c an move this way
 */
int canPieceMove(int piece, char *move)
{
    switch (piece)
    {
        case PIECE_BISHOP:
            if (move[1] - move[0] != move[3] - move[2] &&
                move[1] + move[0] != move[3] + move[2])
            {
                return 0;
            }
            // check if there isn't any other piece in the way
            // TODO
            break;
        case PIECE_KING:
            if (abs(move[0] - move[2]) > 1 || abs(move[1] - move[3]) > 1)
            {
                return 0;
            }
            break;
        case PIECE_KNIGHT:
            if (!((abs(move[3] - move[1]) == 2 && abs(move[0] - move[2]) == 1) || 
                (abs(move[3] - move[1]) == 1 && abs(move[0] - move[2]) == 2)))
            {
                return 0;
            }
            break;
        case PIECE_PAWN:
            // first move of this pawn
            if (move[3] - move[1] == 2 && (move[1] != 1 || move[1] != CHESS_BOARD - 2))
            {
                return 0;
            }
            // TODO: capturing other players piece (diagonal)
            
            if (move[3] - move[1] != 1)
            {
                return 0;
            }
            break;
        case PIECE_QUEEN:
            if (!((move[1] - move[0] == move[3] - move[2] ||
                 move[1] + move[0] == move[3] + move[2]) ||
                (abs(move[2] - move[0]) > 0 && move[3] - move[1] == 0) ||
                (abs(move[3] - move[1]) > 0 && move[2] - move[1] == 0)))
            {
                return 0;
            }
            break;
        case PIECE_ROOK:
            if (!((abs(move[2] - move[0]) > 0 && move[3] - move[1] == 0) ||
                (abs(move[3] - move[1]) > 0 && move[2] - move[1] == 0)))
            {
                return 0;
            }
            break;
    }
    return 1;
}

/**
 * Test if piece can move this way
 * 
 * @param piece
 * @param move
 */
void testPieceMove(int piece, char *move)
{
    printf("\n--- TEST ---\nFigurka: %d\nTah: %s\n", piece, move);
    if (canPieceMove(piece, move))
    {
        printf("LZE\n");
    }
    else
    {
        printf("NELZE\n");
    }
}

void testPieces()
{
    /*
    printf("BISHOP\n");
    testPieceMove(PIECE_BISHOP, "c2e4");
    testPieceMove(PIECE_BISHOP, "c3b3");
    testPieceMove(PIECE_BISHOP, "e4c2");
    
    printf("\n\nKING\n");
    testPieceMove(PIECE_KING, "c2c3");
    testPieceMove(PIECE_KING, "c2d3");
    testPieceMove(PIECE_KING, "c2d1");
    testPieceMove(PIECE_KING, "c2d4");
    testPieceMove(PIECE_KING, "c2a4");
    testPieceMove(PIECE_KING, "c2a2");
     */ 
    /*
    printf("\n\nKNIGHT\n");
    testPieceMove(PIECE_KNIGHT, "c3d5");
    testPieceMove(PIECE_KNIGHT, "c3e4");
    testPieceMove(PIECE_KNIGHT, "c3e2");
    testPieceMove(PIECE_KNIGHT, "c3d1");
    testPieceMove(PIECE_KNIGHT, "c3b1");
    testPieceMove(PIECE_KNIGHT, "c3a2");
    testPieceMove(PIECE_KNIGHT, "c3a4");
    testPieceMove(PIECE_KNIGHT, "c3b5");
     */
    /*
    printf("ne:\n");
    testPieceMove(PIECE_KNIGHT, "c3a5");
    testPieceMove(PIECE_KNIGHT, "c3c5");
    testPieceMove(PIECE_KNIGHT, "c3e5");
    testPieceMove(PIECE_KNIGHT, "c3b4");
    testPieceMove(PIECE_KNIGHT, "c3c4");
    testPieceMove(PIECE_KNIGHT, "c3d4");
    testPieceMove(PIECE_KNIGHT, "c3a3");
    testPieceMove(PIECE_KNIGHT, "c3b3");
    testPieceMove(PIECE_KNIGHT, "c3d3");
    testPieceMove(PIECE_KNIGHT, "c3e3");
    testPieceMove(PIECE_KNIGHT, "c3b2");
    testPieceMove(PIECE_KNIGHT, "c3c2");
    testPieceMove(PIECE_KNIGHT, "c3d2");
    testPieceMove(PIECE_KNIGHT, "c3a1");
    testPieceMove(PIECE_KNIGHT, "c3c1");
    testPieceMove(PIECE_KNIGHT, "c3e1");
     */
    /*
     * Pawns test differently (we must know their color)
    printf("\n\nPAWN\n");
    testPieceMove(PIECE_PAWN, "b2b4");
    testPieceMove(PIECE_PAWN, "b2b3");
    testPieceMove(PIECE_PAWN, "d2d4");
    testPieceMove(PIECE_PAWN, "d5d6");
     * */
    /*
    printf("\n\nQUEEN\n");
     */
    /*
    testPieceMove(PIECE_QUEEN, "2202");
    testPieceMove(PIECE_QUEEN, "2204");
    testPieceMove(PIECE_QUEEN, "2224");
    testPieceMove(PIECE_QUEEN, "2244");
    testPieceMove(PIECE_QUEEN, "2242");
    testPieceMove(PIECE_QUEEN, "2240");
    testPieceMove(PIECE_QUEEN, "2220");
    testPieceMove(PIECE_QUEEN, "2200");
     */
    /*
    testPieceMove(PIECE_QUEEN, "2201");
    testPieceMove(PIECE_QUEEN, "2203");
    testPieceMove(PIECE_QUEEN, "2215");
    testPieceMove(PIECE_QUEEN, "2235");
    testPieceMove(PIECE_QUEEN, "2243");
    testPieceMove(PIECE_QUEEN, "2241");
    testPieceMove(PIECE_QUEEN, "2230");
    testPieceMove(PIECE_QUEEN, "2210");
     */
    
    /*
    printf("\n\nROOK\n");
    /*
    testPieceMove(PIECE_ROOK, "2202");
    testPieceMove(PIECE_ROOK, "2224");
    testPieceMove(PIECE_ROOK, "2242");
    testPieceMove(PIECE_ROOK, "2220");
     */
    /*
    testPieceMove(PIECE_ROOK, "2201");
    testPieceMove(PIECE_ROOK, "2203");
    testPieceMove(PIECE_ROOK, "2215");
    testPieceMove(PIECE_ROOK, "2235");
    testPieceMove(PIECE_ROOK, "2243");
    testPieceMove(PIECE_ROOK, "2241");
    testPieceMove(PIECE_ROOK, "2230");
    testPieceMove(PIECE_ROOK, "2210");
    testPieceMove(PIECE_ROOK, "2204");
    testPieceMove(PIECE_ROOK, "2244");
    testPieceMove(PIECE_ROOK, "2240");
    testPieceMove(PIECE_ROOK, "2200");
     */
    
    while(getchar()!='\n');
    exit(0);
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
    char *ip_address = "10.0.0.139";
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
        number_of_game++;

        first_player_size = sizeof (first_player);
        connected_first = accept(sock, (struct sockaddr *) &first_player, &first_player_size);
        printf("\nGAME %d: First player connected from %s, port %d\n", number_of_game, inet_ntoa(first_player.sin_addr), ntohs(first_player.sin_port));

        sendPlayerCommand(connected_first, COMMAND_STATUS, COMMAND_SUCCESS);
        sendPlayerCommand(connected_first, COMMAND_COLOR, COMMAND_COLOR_WHITE);
        
        // wait for next player, so game can begin
        second_player_size = sizeof (second_player);
        connected_second = accept(sock, (struct sockaddr *) &second_player, &second_player_size);
        printf("\nGAME %d: Second player connected from %s, port %d\n", number_of_game, inet_ntoa(second_player.sin_addr), ntohs(second_player.sin_port));
        
        sendPlayerCommand(connected_second, COMMAND_STATUS, COMMAND_SUCCESS);
        sendPlayerCommand(connected_second, COMMAND_COLOR, COMMAND_COLOR_BLACK);
        
        if (fork() == 0)
        {
            // set white and black player
            printf("GAME %d: Setting white and black player\n", number_of_game);
            struct player white_player, black_player;
            struct chess_game game;
            game.number = number_of_game;
            
            white_player.color = WHITE_COLOR;
            white_player.reference = connected_first;
            white_player.victorious = 0;
            black_player.color = BLACK_COLOR;
            black_player.reference = connected_second;
            black_player.victorious = 0;
            
            // init chessboard
            printf("GAME %d: Initialization of chessboard\n", number_of_game);
            initChessBoard(&game);
            
            printf("GAME %d: Game begins\n", number_of_game);
            int check_mate = 0, check_stalemate = 0;
            game.player = white_player;
            while (!check_mate && !check_stalemate)
            {
                // game loop
                printf("GAME %d: Turn of player %d\n", number_of_game, game.player.reference);
                // sendPlayerCommand(game.player.reference, COMMAND_MESSAGE, "Your move:");
                int moveStatus = 0;
                char *move = (char *)malloc(sizeof(char) * 16);
                while (moveStatus != 1)
                {
                    move = receivePlayerData(game.player.reference);
                    moveStatus = playMove(&game, move);
                }
                printf("GAME %d: End of turn of player %d\n", number_of_game, game.player.reference);
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
