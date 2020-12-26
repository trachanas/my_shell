void printBash();
void printDirectory();

int isPipe(char *);
int isRedirect(char *);
int isSimpleCommand(char *);

void splitCommands(char *, char **);
void resetC(char **);

void execSimpleCommand(char **);

void execPipedCommands(char **, char **);

char* skipwhite(char* );
