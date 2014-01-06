
/**
 * Send player to message
 * 
 * @param connected Reference to connected player
 * @param message Message for player
 */
void sendPlayerMessage(int connected, char *message)
{
    int success = 1;
    char replyBuffer[32];
    sprintf(replyBuffer, "%s\n", message);
    if (send(connected, replyBuffer, strlen(replyBuffer), 0) == -1)
    {
        perror("send() failed");
    }
    success = 0;
}
