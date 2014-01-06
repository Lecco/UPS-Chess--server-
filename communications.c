
/**
 * Send message to player, of message couldn't be sent, this function will
 * print error message to stderr
 * 
 * @param connected Reference to connected player
 * @param command Command for player (for futher info read documentation)
 * @param param Param of command, differs by command
 */
void sendPlayerCommand(int connected, char *command, char *param)
{
    char replyBuffer[32];
    sprintf(replyBuffer, "%s\n", param);
    if (send(connected, replyBuffer, strlen(replyBuffer), 0) == -1)
    {
        perror("send() failed");
    }
}
