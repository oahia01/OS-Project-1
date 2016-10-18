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


typedef struct Environment {
    char *shell;
    char *PWD;
} Environment;

void setupEnvironment(Environment *env);


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

}

void setupEnvironment(Environment *env){
    char envbuf[PATH_MAX];
    if (readlink("/proc/self/exe", envbuf, sizeof(envbuf) - 1) == -1)
        fprintf(stderr, "Cannot get directory of shell executable\n");
    else {
        env->PWD = env->shell = (char *) malloc(1 + strlen("shell=")+ strlen(envbuf));
        strcpy(env->PWD, "shell=");
        strcat(env->PWD, envbuf);
        strcpy(env->shell, env->PWD);
    }
}