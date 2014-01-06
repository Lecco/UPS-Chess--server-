
/**
 * Send message to player, of message couldn't be sent, this function will
 * print error message to stderr
 * 
 * @param connected Reference to connected player
 * @param message Message for player
 */
void sendPlayerMessage(int connected, char *message)
{
    char replyBuffer[32];
    sprintf(replyBuffer, "%s\n", message);
    if (send(connected, replyBuffer, strlen(replyBuffer), 0) == -1)
    {
        perror("send() failed");
    }
}
