#include "functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>


#define YEL   "\x1B[33m"
#define RESET "\x1B[0m"

#define MAX_COM 1024
#define MAX_LIST 10
#define READ 0
#define WRITE 1




// Output: prints the bash prompt
void printBash(){
    printf("myBash$ ");
}


// Output: prints the current directory

void printDirectory(){

    char dir[MAX_COM];

    getcwd(dir, sizeof(dir));

    printf("We are in " YEL "%s" RESET " directory!\n", dir);
}


// Input: string from stdin
// Output: 1 if its a pipe, 0 otherwise
int isPipe(char *string){

    if (strchr(string, '|') != NULL){
        return 1;
    } else{
        return 0;
    }
}


int isRedirect(char *string){

    if ((strchr(string, '>')) || (strchr(string, '<')) != NULL){
        return 1;
    } else{
        return 0;
    }
}

int isSimpleCommand(char *string){

    if (strchr(string, '+') != NULL){
        return 1;
    } else{
        return 0;
    }

}


void splitCommands(char *string, char **cmd){

    char *token = NULL;

    int i = 0;

    token = strtok(string, " ");

    while (token != NULL){

        cmd[i] = strdup(token);

        i++;

        token = strtok(NULL, " ");
    }

    cmd[i] = NULL;


}

void resetC(char **cmd) {
    for (int k = 0; k < MAX_COM; k++){
        cmd[k] = NULL;
    }
}

void execSimpleCommand(char **cmd) {

    int ret_val = 0;

    pid_t pid = fork();

    if (pid == -1){
        printf("Failed creating a child process!\n");
        return;
    }
    else if (pid == 0){

        ret_val = execvp(cmd[0], cmd);
        if (ret_val < 0) {
            printf("\n Cannot execute \n");
        }
    }
    else {
        wait(NULL);
    }

}

void execPipedCommands(char **cmd, char **cmdPiped) {

    int pipeOne[2], status, ret_val, s;

    status = pipe(pipeOne);
    if (status < 0) {
        exit(-1);
    }

    pid_t p1, p2, w;

    p1 = fork();

    if (p1 < 0) {
        printf("Fork failed!\n");
    }

    if (p1 == 0) {

        close(pipeOne[READ]);

        dup2(pipeOne[WRITE], STDOUT_FILENO);

        close(pipeOne[WRITE]);

        if (execvp(cmd[0], cmd) < 0) {
            perror("Lathos");
        }

    } else {
        p2 = fork();

        if (p2 < 0) {
            printf("Fork failed\n");
        }

        if (p2 == 0) {

            close(pipeOne[WRITE]);

            dup2(pipeOne[READ], STDIN_FILENO);

            close(pipeOne[READ]);

            if (execvp(cmdPiped[0], cmdPiped) < 0) {
                perror("Lathos!");
            }
        } else {
            // parent is waiting
            waitpid(-1, &s, WUNTRACED | WCONTINUED);
            printBash();
        }

    }
}

char* skipwhite(char* s)
{
    while (isspace(*s)) ++s;
    return s;
}
