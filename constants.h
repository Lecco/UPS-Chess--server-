/* 
 * File:   constants.h
 * Author: Oldřich Pulkrt <O.Pulkrt@gmail.com>
 *
 * Created on 4. leden 2014, 10:33
 */

#define CHESS_BOARD 8           /* Length of chess board */
#define MOVE_LENGTH 4           /* Length of move (move looks like b1c3) */
#define DEFAULT_COLOR 0         /* This constant is used on fields where aren't pieces */
#define WHITE_COLOR 2           /* Represents white color of chess piece */
#define BLACK_COLOR 1           /* Represents black color of chess piece */
#define PIECE_PAWN 1            /* Represents pawn (= pesec) */
#define PIECE_ROOK 2            /* Represents rook (= vez) */
#define PIECE_KNIGHT 3          /* Represents knight (= jezdec) */
#define PIECE_BISHOP 4          /* Represents bishop (= strelec) */
#define PIECE_QUEEN 5           /* Represents queen (= dama) */
#define PIECE_KING 6            /* Represents king (= kral) */
#define WHITE_PLAYER_INIT 2     /* White player has two first rows */
#define BLACK_PLAYER_INIT 5     /* Black player has rows above fifth */

#define COMMAND_STATUS "STATUS"
#define COMMAND_SUCCESS "SUCCESS"
#define COMMAND_FAIL "FAIL"
#define COMMAND_MOVE "MOVE"
#define COMMAND_GAME_STATUS "GAME_STATUS"
#define COMMAND_GAME_STATUS_CHECK "CHECK"
#define COMMAND_GAME_STATUS_CHECKMATE "CHECKMATE"
#define COMMAND_GAME_STATUS_STALESTATE "STALEMATE"
#define COMMAND_WHITE_STATUS "WHITE_STATUS"
#define COMMAND_CONNECTED "CONNECTED"
#define COMMAND_DISCONNECTED "DISCONNECTED"
#define COMMAND_BLACK_STATUS "BLACK_STATUS"
#define COMMAND_MESSAGE "MESSAGE"

#define ASCII_CAPITAL_A 65
#define ASCII_CAPITAL_H 72
#define ASCII_A 97
#define ASCII_H 104
#define ASCII_0 48
#define ASCII_9 57

#define MOVE_PLAYABLE 1
#define MOVE_NOT_CHESSBOARD 2
#define MOVE_OWN_PIECE 3
#define MOVE_NOT_OWNER 4
#define MOVE_NOT_PLAYABLE 5
