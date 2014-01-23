
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

    /* Info about connection */
    int connected;
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

    /* White player in this game */
    struct player *white_player;

    /* Black player in this game */
    struct player *black_player;

    /* Identifier of game */
    int number;

    /* Current game status, check, checkmate, normal, stalemate */
    int state;

    /* COLOR of checked player */
    int check;
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
    int prevPiece, prevColor, is_check;
    switch (movePlayable)
    {
        case MOVE_NOT_CHESSBOARD:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL, game);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "Piece would end outside chessboard, try again.", game);
            break;
        case MOVE_NOT_OWNER:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL, game);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "You tried to move other players piece.", game);
            break;
        case MOVE_OWN_PIECE:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL, game);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "You tried to move on field with yout piece on it, try again.", game);
            break;
        case MOVE_NOT_PLAYABLE:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL, game);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "Piece can not perform this move.", game);
            break;
        case MOVE_PLAYABLE:
            prevPiece = game->board_figures[move[3]][move[2]];
            prevColor = game->board_colors[move[3]][move[2]];

            game->board_figures[move[3]][move[2]] = game->board_figures[move[1]][move[0]];
            game->board_colors[move[3]][move[2]] = game->board_colors[move[1]][move[0]];
            game->board_figures[move[1]][move[0]] = DEFAULT_COLOR;
            game->board_colors[move[1]][move[0]] = DEFAULT_COLOR;

            is_check = isCheck(game);
            if (is_check == game->player.color)
            {
                // return last move
                game->board_figures[move[1]][move[0]] = game->board_figures[move[3]][move[2]];
                game->board_colors[move[1]][move[0]] = game->board_colors[move[3]][move[2]];

                game->board_figures[move[3]][move[2]] = prevPiece;
                game->board_colors[move[3]][move[2]] = prevColor;

                sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL, game);
                sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "This move would end up as check for you.", game);
                return 0;
            }
            else if (is_check == WHITE_COLOR || is_check == BLACK_COLOR)
            {
                game->state = STATUS_CHECK;
                game->check = (game->player.color == WHITE_COLOR ? BLACK_COLOR : WHITE_COLOR);
            }

            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_SUCCESS, game);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "Move successfully completed.", game);
            break;
        default:
            sendPlayerCommand(game->player.reference, COMMAND_STATUS, COMMAND_FAIL, game);
            sendPlayerCommand(game->player.reference, COMMAND_MESSAGE, "Incorrect move.", game);
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
    int other_color;
    if (game->board_colors[move[1]][move[0]] == game->board_colors[move[3]][move[2]])
    {
        return 0;
    }
    switch (piece)
    {
        case DEFAULT_COLOR:
            return 0;
            break;
        case PIECE_BISHOP:
            if (move[1] - move[0] != move[3] - abs(move[2]))
            {
                return 0;
            }
            // check if there isn't any other piece in the way
            if (!isPathFree(game, move))
            {
                return 0;
            }
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
            other_color = (game->player.color == WHITE_COLOR ? BLACK_COLOR : WHITE_COLOR);
            if (!((move[2] - move[0] == 0) && (((move[3] - move[1] == 2 || move[3] - move[1] == 1) && game->player.color == WHITE_COLOR) ||
                        ((move[1] - move[3] == 2 || move[1] - move[3] == 1) && game->player.color == BLACK_COLOR))) ||
                    game->board_colors[move[3]][move[2]] == other_color)
            {
                return 0;
            }
            break;
        case PIECE_QUEEN:
            if (!((move[1] - move[0] == move[3] - move[2] ||
                 move[1] + move[0] == move[3] + move[2]) ||
                (abs(move[2] - move[0]) > 0 && move[3] - move[1] == 0) ||
                (abs(move[3] - move[1]) > 0 && move[2] - move[0] == 0)))
            {
                return 0;
            }
            if (!isPathFree(game, move))
            {
                return 0;
            }
            break;
        case PIECE_ROOK:
            if (!((abs(move[2] - move[0]) > 0 && abs(move[3] - move[1]) == 0) ||
                (abs(move[3] - move[1]) > 0 && abs(move[2] - move[0]) == 0)))
            {
                return 0;
            }
            if (!isPathFree(game, move))
            {
                return 0;
            }
            break;
    }
    return 1;
}

/**
 * Check if there is any piece between starting and ending position
 *
 * @param game Game with info about positions and colors
 * @param move Move we want to perform
 * @return True if there isn't any piece in path
 */
int isPathFree(struct chess_game *game, char *move)
{
    int x1 = move[0];
    int y1 = move[1];
    int x2 = move[2];
    int y2 = move[3];

    int i, j;
    for (i = 0; i < CHESS_BOARD; i++)
    {
        for (j = 0; j < CHESS_BOARD; j++)
        {
            if ((i == y1 && j == x1) ||
                (i == y2 && j == x2) ||
                game->board_colors[i][j] == DEFAULT_COLOR)
            {
                continue;
            }
            if (isBetweenPoints(j, i, x1, y1, x2, y2))
            {
                return 0;
            }
        }
    }
    return 1;
}

/**
 * Check whether point is between other two (if it lies on line between them
 *
 * @param x X coordinate of point
 * @param y Y coordinate of point
 * @param x1 Coordinate of start point
 * @param y1 Coordinate of start point
 * @param x2 Coordinate of ending point
 * @param y2 Coordinate of ending point
 */
int isBetweenPoints(int x, int y, int x1, int y1, int x2, int y2)
{
    int lineX = y2 - y1;
    int lineY = x1 - x2;
    int c = -lineX * x1 - lineY * y1;
    if (lineX * x + lineY * y + c == 0 &&
        ((x > x1 && x < x2) || (x > x2 && x < x1) ||
         (y > y1 && y < y2) || (y > y2 && y < y1)))
    {
        return 1;
    }
    return 0;
}

/**
 * Check if there is check and on which player
 *
 * @param game Chessboard with figures, position and their colors
 * @return Color of checked player
 */
int isCheck(struct chess_game *game)
{
    // find kings
    int i, j;
    int king_x, king_y, king_x2, king_y2;
    for (i = 0; i < CHESS_BOARD; i++)
    {
        for (j = 0; j < CHESS_BOARD; j++)
        {
            if (game->board_figures[i][j] == PIECE_KING && game->board_colors[i][j] == game->player.color)
            {
                king_x = j;
                king_y = i;
            }
            if (game->board_figures[i][j] == PIECE_KING && game->board_colors[i][j] != game->player.color)
            {
                king_x2 = j;
                king_y2 = i;
            }
        }
    }
    // find out if any pieces of other player can move to king
    char *move = (char *)malloc(sizeof(char) * 16);
    for (i = 0; i < CHESS_BOARD; i++)
    {
        for (j = 0; j < CHESS_BOARD; j++)
        {
            if (game->board_figures[i][j] == DEFAULT_COLOR || game->board_colors[i][j] == game->player.color)
            {
                continue;
            }
            move[0] = j;
            move[1] = i;
            move[2] = king_x;
            move[3] = king_y;
            if (pieceMove(game, move))
            {
                return game->player.color;
            }
        }
    }
    // if check for other player?
    for (i = 0; i < CHESS_BOARD; i++)
    {
        for (j = 0; j < CHESS_BOARD; j++)
        {
            if (game->board_figures[i][j] != DEFAULT_COLOR && game->board_colors[i][j] != game->player.color)
            {
                continue;
            }
            move[0] = j;
            move[1] = i;
            move[2] = king_x2;
            move[3] = king_y2;
            if (pieceMove(game, move))
            {
                return (game->player.color == WHITE_COLOR ? BLACK_COLOR : WHITE_COLOR);
            }
        }
    }
    return 0;
}

/**
 * Copy game progress from given chess game (so we can try moves on a copy
 *
 * @param game Chess game we want to copy
 * @return Copy of chess game
 */
struct chess_game *copyChessGame(struct chess_game *game)
{
    struct chess_game *g;
    int i, j;

    g = (struct chess_game *) malloc(sizeof(struct chess_game) + sizeof(int *) * sizeof(int *) * sizeof(int *) * sizeof(int *) * CHESS_BOARD * CHESS_BOARD);
    if (g == NULL)
    {
        return NULL;
    }

    g->number = game->number;
    g->player = game->player;
    g->board_figures = (int **) malloc(sizeof(int *) * sizeof(int *) * CHESS_BOARD);
    g->board_colors = (int **) malloc(sizeof(int *) * sizeof(int *) * CHESS_BOARD);

    for (i = 0; i < CHESS_BOARD; i++)
    {
        g->board_figures[i] = (int *) malloc(sizeof(int *) * CHESS_BOARD);
        g->board_colors[i] = (int *) malloc(sizeof(int *) * CHESS_BOARD);
        for (j = 0; j < CHESS_BOARD; j++)
        {
            g->board_figures[i][j] = game->board_figures[i][j];
            g->board_colors[i][j] = game->board_colors[i][j];
        }
    }

    return g;
}

int isCheckmate(struct chess_game *game)
{
    if (game->check == STATUS_DEFAULT)
    {
        return 0;
    }
    int i, j, k, l;
    char *move = (char *) malloc(sizeof(char) * 16);
    struct chess_game *temp;
    for (i = 0; i < CHESS_BOARD; i++)
    {
        for (j = 0; j < CHESS_BOARD; j++)
        {
            if (game->check == game->board_colors[i][j])
            {
                for (k = 0; k < CHESS_BOARD; k++)
                {
                    for (l = 0; l < CHESS_BOARD; l++)
                    {
                        temp = copyChessGame(game);
                        if (temp->player.color == WHITE_COLOR)
                        {
                            temp->player.color = BLACK_COLOR;
                        }
                        else
                        {
                            temp->player.color = WHITE_COLOR;
                        }
                        move[0] = j;
                        move[1] = i;
                        move[2] = l;
                        move[3] = k;
                        if (!pieceMove(temp, move))
                        {
                            continue;
                        }
                        temp->board_figures[move[3]][move[2]] = temp->board_figures[move[1]][move[0]];
                        temp->board_colors[move[3]][move[2]] = temp->board_colors[move[1]][move[0]];
                        temp->board_figures[move[1]][move[0]] = DEFAULT_COLOR;
                        temp->board_colors[move[1]][move[0]] = DEFAULT_COLOR;
                        if (isCheck(temp) == 0)
                        {
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return 1;
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
    char ip_address[strlen(argv[1])];
    strcpy(ip_address, argv[1]);
    int port = atoi(argv[2]);
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

        sendConnectionInfo(connected_first, COMMAND_STATUS, COMMAND_SUCCESS);
        sendConnectionInfo(connected_first, COMMAND_COLOR, COMMAND_COLOR_WHITE);
        
        // wait for next player, so game can begin
        second_player_size = sizeof (second_player);
        connected_second = accept(sock, (struct sockaddr *) &second_player, &second_player_size);
        printf("\nGAME %d: Second player connected from %s, port %d\n", number_of_game, inet_ntoa(second_player.sin_addr), ntohs(second_player.sin_port));
        
        sendConnectionInfo(connected_second, COMMAND_STATUS, COMMAND_SUCCESS);
        sendConnectionInfo(connected_second, COMMAND_COLOR, COMMAND_COLOR_BLACK);

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
            white_player.victorious = 0;
            white_player.connected = 1;
            black_player.color = BLACK_COLOR;
            black_player.reference = connected_second;
            black_player.victorious = 0;
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
                    strcpy(receivedMove,  move);
                    move_status = playMove(&game, move);
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
