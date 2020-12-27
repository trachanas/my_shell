#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define YEL   "\x1B[33m"
#define RESET "\x1B[0m"

#define MAX_COM 1024


int main(int argc, char **argv){

    char userInput[MAX_COM];
    char *c1 = NULL;
    char *c2 = NULL;
    char *token = NULL;

    int f;
    printDirectory();
    printf("\n\n");
    printf("|---------------------------------|\n");
    printf("|             HELLO               |\n");
    printf("|---------------------------------|\n");
    printf("\n\n\n\n");
    int f1, f2;

    printBash();
    char *first, *file1, *file2;

    char **cmd = calloc(MAX_COM, sizeof(char *));
    for (int i = 0; i < MAX_COM; i++){
        cmd[i] = calloc(MAX_COM, sizeof(char));
    }


    char **cmdPipe = calloc(MAX_COM, sizeof(char *));
    for (int i = 0; i < MAX_COM; i++){
        cmdPipe[i] = calloc(MAX_COM, sizeof(char));
    }
    int countPipes = 0;

    char *t;
    char *c;

    while(fgets(userInput, MAX_COM, stdin) != NULL){

        if (strcmp(userInput, "\n") == 0){
            printBash();
            continue;
        }

        userInput[strlen(userInput) - 1] = '\0';

        if (strcmp(userInput, "exit") == 0){

            printf("Goodbye!\n");

            return EXIT_SUCCESS;
        }

        if (isPipe(userInput)) {
            countPipes = 0;
            for (int l = 0; userInput[l] != '\0'; l++){
                if (userInput[l] == 124){
                    countPipes++;
                }
            }

            if (countPipes == 1 && strstr(userInput, ">>") != NULL){
                token = NULL;

                resetC(cmd);

                resetC(cmdPipe);

                token = strtok(userInput, ">>");

                char *piped = strdup(token);

                token = strtok(NULL, ">>");

                char *file = strdup(token);

                file = skipwhite(file);

                token = strtok(piped, "|");

                c1 = strdup(token);

                token = strtok(NULL, "|");

                c2 = strdup(token);
                c2 = skipwhite(c2);


                splitCommands(c1, cmd);

                splitCommands(c2, cmdPipe);
                int s = dup(1);
                execPipedCommandsRed(cmd, cmdPipe, file);
                dup2(s,1);
                close(s);
                memset(userInput, '\0', 1000);
            }
            else if (countPipes == 1 && strstr(userInput, ">") != NULL){
                token = NULL;

                resetC(cmd);

                resetC(cmdPipe);

                token = strtok(userInput, ">");

                char *piped = strdup(token);

                token = strtok(NULL, ">");

                char *file = strdup(token);

                file = skipwhite(file);

                token = strtok(piped, "|");

                c1 = strdup(token);

                token = strtok(NULL, "|");

                c2 = strdup(token);

                c2 = skipwhite(c2);

                splitCommands(c1, cmd);
                int s2 = dup(1);
                splitCommands(c2, cmdPipe);

                execPipedCommandsWithRed(cmd, cmdPipe, file);
                dup2(s2, 1);
                close(s2);
                memset(userInput, '\0', 1000);
            }
            else if (countPipes == 1) {

                token = NULL;

                resetC(cmd);

                resetC(cmdPipe);

                token = strtok(userInput, "|");

                c1 = strdup(token);

                token = strtok(NULL, "|");

                c2 = strdup(token);

                splitCommands(c1, cmd);

                splitCommands(c2, cmdPipe);

                execPipedCommands(cmd, cmdPipe);

                memset(userInput, '\0', 1000);
            }

        }
        else if (isRedirect(userInput)) {

            resetC(cmd);

            // C1 < C2 >> C3
            if (strstr(userInput, "<") != NULL && strstr(userInput, ">>") != NULL){

                token = strtok(userInput, "<");

                first = strdup(token);

                token = strtok(NULL, ">>");

                file1 = strdup(token);

                token = strtok(NULL, ">>");

                file2 = strdup(token);

                file1 = skipwhite(file1);

                file2 = skipwhite(file2);

                file1[strlen(file1) -1] = '\0';

                f1 = open(file1, O_RDONLY, 0644);
                if (f1 < 0){
                    puts("error f1");
                }
                f2 = open(file2, O_WRONLY| O_APPEND | O_CREAT, 0644);
                if (f2 < 0) {
                    puts("error f2");

                }
                int s1 = dup(0);

                dup2(f1, 0);
                close(f1);

                int s2 = dup(1);

                dup2(f2, 1);
                close(f2);

                resetC(cmd);

                splitCommands(first, cmd);

                execSimpleCommand(cmd);

                dup2(s1, 0);
                close(s1);

                dup2(s2, 1);
                close(s2);
                printBash();

            }

            // C1 < C2 > C3
            else if (strstr(userInput, "<") != NULL && strstr(userInput, ">") != NULL){

                token = strtok(userInput, "<");

                first = strdup(token);

                token = strtok(NULL, ">");

                file1 = strdup(token);

                token = strtok(NULL, ">");

                file2 = strdup(token);

                file1 = skipwhite(file1);
                file2 = skipwhite(file2);

                file1[strlen(file1) -1] = '\0';

                f1 = open(file1, O_RDONLY, 0644);
                if (f1 < 0){
                    puts("error f1");
                }
                f2 = open(file2, O_CREAT | O_WRONLY | O_TRUNC, 0644);
                if (f2 < 0) {
                    puts("error f2");
                }

                int s1 = dup(0);
                dup2(f1, 0);
                close(f1);

                int s2 = dup(1);
                dup2(f2, 1);
                close(f2);


                resetC(cmd);

                splitCommands(first, cmd);

                execSimpleCommand(cmd);

                dup2(s1, 0);
                close(s1);
                dup2(s2, 1);
                close(s2);

                printBash();

            }
            // C1 > C2
            else if (strstr(userInput, ">") != NULL && strstr(userInput, ">>") == NULL){

                resetC(cmd);

                t = strtok(userInput, ">");

                c = strdup(t);

                t = strtok(NULL, ">");

                char *file = strdup(t);

                splitCommands(c,cmd);

                file = skipwhite(file);

                f = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);

                int save = dup(1);
                dup2(f,1);

                close(f);

                execSimpleCommand(cmd);

                dup2(save, 1);
                close(save);
                printBash();
            }

            // C1 >> C2
            else if (strstr(userInput, ">>") != NULL && strstr(userInput, "<") == NULL){
                resetC(cmd);

                t = strtok(userInput, ">>");

                c = strdup(t);

                t = strtok(NULL, ">>");

                char *file = strdup(t);

                splitCommands(c,cmd);

                file = skipwhite(file);

                f = open(file, O_WRONLY| O_APPEND | O_CREAT, 0600);

                int save = dup(1);

                dup2(f,1);

                close(f);

                execSimpleCommand(cmd);

                dup2(save, 1);

                close(save);

                printBash();

            }

            // C1 < C2
            else if (strstr(userInput, "<") != NULL && strstr(userInput, "<<") == NULL) {
                resetC(cmd);

                t = strtok(userInput, "<");

                c = strdup(t);

                t = strtok(NULL, "<");

                char *file = strdup(t);

                splitCommands(c,cmd);

                file = skipwhite(file);

                f = open(file, O_RDONLY, 06444);

                int save = dup(0);

                dup2(f, 0);

                close(f);

                execSimpleCommand(cmd);
                dup2(save, 0);
                close(save);
                printBash();
            }

        }
        else {
            resetC(cmd);

            splitCommands(userInput, cmd);

            execSimpleCommand(cmd);

            memset(userInput, '\0', 1000);

            printBash();
        }

    }
    return EXIT_SUCCESS;
}