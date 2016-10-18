/*
 * Date created: 10/18/2016
 * By: 
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>


const char *whites = " \t\n";

typedef struct Environment {
    char *shell;
    char *PWD;
} Environment;

void setupEnvironment(Environment *env);
// void cleanupString(char *line, size_t *lineLen);
void runCommand(char **argv, int argc);


int main(int argc, char *argv[]) {
    
    int batchMode = 0;
    FILE *batchFile;
    Environment env;

    if (argc == 1) {
        batchMode = 0;
    } else if (argc == 2) {
        batchFile = fopen(argv[1], "r");
        if (batchFile == NULL) {
            fprintf(stderr, "Error reading from batch file: errno = %d\n", errno);
            exit(1);
        }
        batchMode = 1;
    } else {
        fprintf(stderr, "Error: you must provide only one batch file, or no batch "
                                "files to run in interactive mode.\n");
        exit(1);
    }
    
    setupEnvironment(&env);

    char *line;
    size_t buffSize = 32;
    size_t lineLen;
    line = (char *)malloc(buffSize * sizeof(char));    

    int halt = 0;
    while (!halt) {

        if (!batchMode) printf(">> ");
        lineLen = getline(&line, &buffSize, (batchMode) ? batchFile : stdin);
        
        /* Unwrap sequence of commands delimited by ; */
        char **cmds;
        cmds = malloc(lineLen/2 * sizeof(char*));    
        cmds[0] = strtok(line, ";");
        int i = 1;
        do {
            cmds[i] = strtok(NULL, ";");
        } while (cmds[i++] != NULL);
        int numCmds = i-1;

        for (int i = 0; i < numCmds && !halt; i++) {
            
            /* Handle empty cases. */
            if (!strcmp(cmds[i], "")) continue;

            /* Unwrap each argument delimited by whitespaces */
            char **argv;
            argv = malloc(lineLen/2 * sizeof(char*));    
            argv[0] = strtok(cmds[i], whites);
            int j = 1;
            do {
                argv[j] = strtok(NULL, whites);
            } while (argv[j++] != NULL);
            int argc = j-1;

            /* Handle quit and EOF. */
            if (!strcmp(argv[0], "quit") || (batchMode && feof(batchFile))) {
                halt = 1;
                free(argv);
                break;
            }

            runCommand(argv, argc);

            free(argv);
        }

        free(cmds);
    }
    
    free(line);
    if (batchMode) fclose(batchFile);
    exit(0);

}


void setupEnvironment(Environment *env) {
    char envbuf[PATH_MAX];
    if (readlink("/proc/self/exe", envbuf, sizeof(envbuf) - 1) == -1)
        fprintf(stderr, "Cannot get directory of shell executable\n");
    else {
        char *path = (char *) malloc(1 + strlen("shell=") + strlen(envbuf));
        strcpy(path, "shell=");
        strcat(path, envbuf);
        strcpy(env->shell, path);
        strcpy(env->shell, env->PWD);
        free(path);
    }
}


void runCommand(char **argv, int argc) {

    for (int i = 0; i < argc; i++)
        fprintf(stderr, "%s - ", argv[i]);
    fprintf(stderr, "\n");

}







// void cleanupString(char *line, size_t *lineLen) {

//     /* Clean trailing white spaces: */
//     while (*lineLen > 0 && isspace(line[*lineLen-1]))
//         (*lineLen)--;
    
//     /* Clean leading white spaces: */
//     int leadingWhites = 0;
//     while (leadingWhites < *lineLen && isspace(line[leadingWhites]))
//         leadingWhites++;
//     *lineLen -= leadingWhites;

//     for (int i = 0; i < *lineLen; i++) {
//         line[i] = line[i+leadingWhites];
//     }

//     line[*lineLen] = 0;  /* null-terminate */

// }