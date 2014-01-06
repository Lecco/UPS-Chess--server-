
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
}

