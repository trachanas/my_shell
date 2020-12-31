#include "functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_COM 1024
#define READ 0
#define WRITE 1


// Output: prints the current directory

void printDirectory(){

    char dir[MAX_COM];

    getcwd(dir, sizeof(dir));

    printf("%s/", dir);
}


// Output: prints the bash prompt
void printBash(){
    printDirectory();

    printf("myBash >>>  ");
}

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


// Input: a simple command from stdin
// Output: an array with the command fr arr = {"ls", "-l", NULL}
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


// Input: a piped command from stdin
// Output: an array with the command fr arr = {"ls -l", "grep .c", NULL}
void splitPipes(char **cmd, int *nCom, char *string){

    char *token = strtok(string, "|");

    int i = 0;

    while(token != NULL){

        cmd[i] = strdup(token);

        cmd[i] = skipwhite(cmd[i]);

        i++;

        token = strtok(NULL, "|");
    }

    cmd[i] = NULL;

    *nCom = i;
}



// Input: a set of piped commands, like C1 | C2 |...| Cn and the number of |
// Output: execution of the piped commands
void execMultipleCommands(char *userInput, int count){

    int fd[10][2], nCom;

    pid_t pid;

    char **cmd = calloc(MAX_COM, sizeof(char *));
    for (int i = 0; i < MAX_COM; i++){
        cmd[i] = calloc(MAX_COM, sizeof(char));
    }
    splitPipes(cmd, &nCom, userInput);

    char **cmd1 = calloc(MAX_COM, sizeof(char *));
    for (int i = 0; i < MAX_COM; i++){
        cmd1[i] = calloc(MAX_COM, sizeof(char));
    }


    for (int i = 0; i < count; i++){

        splitCommands(cmd[i], cmd1);

        if(i != count - 1){
            if(pipe(fd[i]) < 0){
                perror("Error with pipe!");

                return;
            }
        }

        pid = fork();

        if (pid == -1){
            perror("Error with fork!");
            return;
        }

        if (pid == 0){
            if(i != count - 1){
                dup2(fd[i][1],1);

                close(fd[i][0]);

                close(fd[i][1]);
            }
            if(i != 0){
                dup2(fd[i-1][0],0);

                close(fd[i-1][1]);

                close(fd[i-1][0]);
            }

            execvp(cmd1[0],cmd1);

            perror("Error with exec!");
            return;
        }

        if(i != 0){
            close(fd[i-1][0]);

            close(fd[i-1][1]);
        }
        wait(NULL);
    }
}



void execSimpleCommand(char **cmd) {

    pid_t pid = fork();

    if (pid == -1){
        perror("Error with fork!");
        return;
    }
    else if (pid == 0){

        execvp(cmd[0], cmd);

        perror("Error with exec!\n");

        return;
    }
    else {
        wait(NULL);
    }
}


// C1 | C2
void execPipedCommands(char **cmd, char **cmdPiped) {

    int pipeOne[2], status, s;

    status = pipe(pipeOne);
    if (status < 0) {
        perror("Error with pipe!\n");

        return;
    }

    pid_t p1, p2;

    p1 = fork();

    if (p1 == -1){
        perror("Error with fork!");
        return;
    }

    if (p1 == 0) {

        close(pipeOne[READ]);

        dup2(pipeOne[WRITE], STDOUT_FILENO);

        close(pipeOne[WRITE]);

        execvp(cmd[0], cmd);

        perror("Error with exec!");
        return;

    } else {
        p2 = fork();

        if (p2 == -1){
            perror("Error with fork!");
            return;
        }

        if (p2 == 0) {

            close(pipeOne[WRITE]);

            dup2(pipeOne[READ], STDIN_FILENO);

            close(pipeOne[READ]);

            execvp(cmdPiped[0], cmdPiped);

            perror("Error with exec!");
            return;

        } else {

            close(pipeOne[0]);
            close(pipeOne[1]);
            wait(NULL);
        }

    }
}


// C1 | C2 >> FILE
void execPipedCommandsRed(char **cmd, char **cmdPiped, char *file){
    int pipeOne[2], status, ret_val, s, k;

    status = pipe(pipeOne);
    if (status < 0) {
        perror("Error with pipe!");
        return;
    }


    pid_t p1, p2;
    p1 = fork();

    if (p1 == -1){
        perror("Error with fork!");
        return;
    }

    if (p1 == 0) {

        close(pipeOne[READ]);

        dup2(pipeOne[WRITE], STDOUT_FILENO);

        close(pipeOne[WRITE]);

        execvp(cmd[0], cmd);

        perror("Error with exec!");
        return;

    } else {
        p2 = fork();

        if (p2 == -1){
            perror("Error with fork!");
            return;
        }


        if (p2 == 0) {

            close(pipeOne[WRITE]);

            dup2(pipeOne[READ], STDIN_FILENO);

            close(pipeOne[READ]);

            k = open(file, O_WRONLY| O_APPEND | O_CREAT, 0644);

            if (k < 0) {
                perror("Error with file k!");
                return;
            }

            dup2(k, 1);
            close(k);

            execvp(cmdPiped[0], cmdPiped);

            perror("Error with exec!");
            return;
        } else {

            close(pipeOne[0]);
            close(pipeOne[1]);
            wait(NULL);

        }

    }
}

// C1 | C2 > FILE
void execPipedCommandsWithRed(char **cmd, char **cmdPiped, char *file){
    int pipeOne[2], status, k;

    status = pipe(pipeOne);
    if (status < 0) {
        perror("Error with pipe!");
        return;
    }
    pid_t p1, p2;

    p1 = fork();

    if (p1 == -1){
        perror("Error with fork!");
        return;
    }

    if (p1 == 0) {

        close(pipeOne[READ]);

        dup2(pipeOne[WRITE], STDOUT_FILENO);

        close(pipeOne[WRITE]);

        execvp(cmd[0], cmd);

        perror("Error with exec!");

        return;

    } else {

        p2 = fork();

        if (p2 == -1){
            perror("Error with fork!");
            return;
        }

        if (p2 == 0) {

            close(pipeOne[WRITE]);

            dup2(pipeOne[READ], STDIN_FILENO);

            close(pipeOne[READ]);

            k = open(file, O_WRONLY | O_CREAT, 0644);
            if (k < 0) {
                perror("Error with file k!");
                return;
            }

            dup2(k, 1);
            close(k);

            execvp(cmdPiped[0], cmdPiped);

            perror("Error with exec!");
            return;
        } else {
            close(pipeOne[0]);
            close(pipeOne[1]);
            wait(NULL);
        }
    }
}

// Input: a string
// Output: cut the whitespaces
char* skipwhite(char* s)
{
    while (isspace(*s)) ++s;
    return s;
}
