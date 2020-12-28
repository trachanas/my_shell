void printBash();
void printDirectory();

int isPipe(char *);
int isRedirect(char *);

void splitCommands(char *, char **);
void resetC(char **);

void execSimpleCommand(char **);

void execPipedCommands(char **, char **);

char* skipwhite(char* );
void execPipedCommandsRed(char **, char **, char *);
void execPipedCommandsWithRed(char **, char **, char *);
void execPipe(char **, char **);
