
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
