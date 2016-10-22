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
#include <sys/stat.h>
#include <string.h>
#include <signal.h>

#define PATH_MAX 4096 

const char *whites = " \t\n";
static volatile int halt = 0;

typedef struct Environment {
    char *shell;
    char *PWD;
} Environment;

void sigintHandler(int sig) { halt = 1; }

void setupEnvironment(Environment *env);
void cleanupString(char *line);
void runCommand(char **argv, int argc);

int main(int argc, char *argv[]) {

    signal(SIGHUP, sigintHandler);

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
    
    // setupEnvironment(&env);

    char *line;
    size_t buffSize = 100;
    size_t lineLimit;
    line = (char *)malloc(buffSize * sizeof(char));    

    while (!halt) {

        if (batchMode && feof(batchFile)) break;

        if (!batchMode) printf(">> ");
        lineLimit = getline(&line, &buffSize, (batchMode) ? batchFile : stdin);

        cleanupString(line);
        if (!strcmp(line, "")) continue;
        // fprintf(stderr, "%s\n\n", line);

        /* Unwrap sequence of commands delimited by ; */
        char **cmds;
        cmds = malloc(lineLimit * sizeof(char*));    
        cmds[0] = strtok(line, ";");
        int i = 1;
        do {
            cmds[i] = strtok(NULL, ";");
        } while (cmds[i++] != NULL);
        int numCmds = i-1;

        for (i = 0; i < numCmds && !halt; i++) {
            
            /* Handle empty cases */
            cleanupString(cmds[i]);
            if (!strcmp(cmds[i], "")) continue;

            /* Unwrap each argument delimited by whitespaces */
            char **argv;
            argv = malloc(lineLimit * sizeof(char*));    
            argv[0] = strtok(cmds[i], whites);
            int j = 1;
            do {
                argv[j] = strtok(NULL, whites);
            } while (argv[j++] != NULL);
            int argc = j-1;

            /* Handle quit */
            if (!strcmp(argv[0], "quit")) {
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
    ssize_t lenBuf;
    if ((lenBuf = readlink("/proc/self/exe", envbuf, sizeof(envbuf) - 1)) == -1)
        fprintf(stderr, "Cannot get directory of shell executable\n");
    else {
        char *path = (char *) malloc(1 + strlen("shell=") + strlen(envbuf));
        strcpy(path, "shell=");
        strcat(path, envbuf);
        printf("%s\n", envbuf);
        printf("%s\n", path);
        // env->shell = (char *) malloc(sizeof(path));
        // env->PWD = (char *) malloc(sizeof(path));
        // strcpy(env->shell, path);
        // strcpy(env->PWD, path);
        free(path);
    }
}

void runCommand(char **argv, int argc) {

    int i;
    for (i = 0; i < argc; i++)
        fprintf(stderr, "%s - ", argv[i]);
    fprintf(stderr, "\n");

}

void cleanupString(char *line) {

    int lineLen = 0;   /* get length of line */
    while (line[lineLen]) lineLen++;

    /* Clean trailing white spaces and ;'s: */
    while (lineLen > 0 && (isspace(line[lineLen-1]) || line[lineLen-1] == ';'))
        lineLen--;
    
    /* Clean leading white spaces and ;'s: */
    int leadingWhites = 0;
    while (leadingWhites < lineLen && (isspace(line[leadingWhites]) || line[leadingWhites] == ';'))
        leadingWhites++;
    lineLen -= leadingWhites;

    int i;
    for (i = 0; i < lineLen; i++) {
        line[i] = line[i+leadingWhites];
    }

    line[lineLen] = 0;  /* null-terminate */

}