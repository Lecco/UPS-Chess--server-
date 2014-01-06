
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
    int **BOARD_FIGURES;
    
    /* In this array are color of figure on fields */
    int **BOARD_COLORS;
};

void initChessBoard(struct chess_game *game)
{

}
