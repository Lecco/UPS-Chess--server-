/* 
 * This file contains functions for playing chess (making moves, checking if there
 * is check, checkmate etc)
 * 
 * File:   chess.c
 * Author: Oldřich Pulkrt <O.Pulkrt@gmail.com>
 *
 * Created on 25. leden 2014, 10:57
 */

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
            if (abs(move[3] - move[1]) != abs(move[2] - abs(move[0])))
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
