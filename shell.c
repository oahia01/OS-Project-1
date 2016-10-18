/*
 * Date created: 10/18/2016
 * By: 
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

typedef struct Environment {
    char *shell;
    char *PWD;
} Environment;

void setupEnvironment(void *fInfoVoid);
void cleanupInput(char *line, size_t *lineLen);
void runCommand(char *cmd, Environment env);

int main(int argc, char *argv[]) {
    
    int batchMode = 0;
    FILE *batchFile;

    if (argc == 1) {
        batchMode = 0;
    } else if (argc == 2) {
        batchFile = fopen(argv[1], "r");
        if (batchFile == NULL) {
            fprintf(stderr, "Error reading from batch file: errno = %d\n", errno);
            if (errno == 2) fprintf(stderr, "No such file or directory.\n");
            exit(1);
        }
        batchMode = 1;
    } else {
        fprintf(stderr, "Error: you must provide only one batch file, or no batch "
                                "files to run in interactive mode.\n");
        exit(1);
    }
    
    Environment env = setupEnvironment();

    char *line;
    size_t buffSize = 32;
    size_t lineLen;
    line = (char *)malloc(buffSize * sizeof(char));    

    while (1) {

        if (!batchMode) printf(">> ");
        lineLen = getline(&line, &buffSize, (batchMode) ? batchFile : stdin);
        cleanupInput(line, &lineLen);
        
        if (lineLen == 0) continue;
        if (batchMode && feof(batchFile)) break;
        if (!strcmp(line, "quit")) break;
        
        runCommand(line, lineLen, env);

        // printf("%zu characters were read.\n", lineLen);
        // printf("Command: %s\n", line);

    }
    
    free(line);
    if (batchMode) fclose(batchFile);
    exit(0);

}


void cleanupString(char *line, size_t *lineLen) {

    /* Clean trailing white spaces: */
    while (*lineLen > 0 && isspace(line[*lineLen-1]))
        (*lineLen)--;
    
    /* Clean leading white spaces: */
    int leadingWhites = 0;
    while (leadingWhites < *lineLen && isspace(line[leadingWhites]))
        leadingWhites++;
    *lineLen -= leadingWhites;

    for (int i = 0; i < *lineLen; i++) {
        line[i] = line[i+leadingWhites];
    }

    line[*lineLen] = 0;  /* null-terminate */

}


void runCommand(char *cmd, size_t cmdLen, Environment env) {

    char *cmdName;
    int cmdNameLen = 0;
    char *args;
    int argsLen = 0;

    while (cmdNameLen < lineLen && cmd[cmdNameLen] != ' ') {
        cmdNameLen++;
    }
    cmd[cmdNameLen] = 0;   /* null-terminate */
    args = &cmd[cmdNameLen+1];
    argsLen = cmdLen - cmdNameLen - 1;

    printf("%s - %s", cmdName, args);

    // if (!strcmp(cmdName, "echo")) {
        // echo(cmd);
    // } 

}




