/* 
 * This file contains structures for playing chess
 * 
 * File:   chess_game.h
 * Author: Oldřich Pulkrt <O.Pulkrt@gmail.com>
 *
 * Created on 25. leden 2014, 10:51
 */

#ifndef CHESS_GAME_H
#define	CHESS_GAME_H
/**
 * Structure player consists of player attributes like color of his pieces, 
 * integer reference on his connection.
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
#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* CHESS_GAME_H */

