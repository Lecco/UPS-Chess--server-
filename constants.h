/* 
 * File:   constants.h
 * Author: Oldřich Pulkrt <O.Pulkrt@gmail.com>
 *
 * Created on 4. leden 2014, 10:33
 */

#define CHESS_BOARD 8           /* Length of chess board */
#define WHITE_CHESSPIECE 1      /* Represents white color of chess piece */
#define BLACK_CHESSPIECE 0      /* Represents black color of chess piece */
#define PIECE_PAWN 0            /* Represents pawn (= pesec) */
#define PIECE_ROOK 1            /* Represents rook (= vez) */
#define PIECE_KNIGHT 2          /* Represents knight (= jezdec) */
#define PIECE_BISHOP 3          /* Represents bishop (= strelec) */
#define PIECE_QUEEN 4           /* Represents queen (= dama) */
#define PIECE_KING 5            /* Represents king (= kral) */
#define WHITE_PLAYER 1
#define BLACK_PLAYER 0

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
