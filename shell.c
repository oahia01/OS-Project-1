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

typedef struct Environment {
    char *shell;
    char *PWD;
} Environment;

void *countWords(void *fInfoVoid);
void setupEnvironment(void *fInfoVoid);


int main(int argc, char *argv[]) {
    
    int batchMode = 0;
    FILE *batchFile;

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
    
    setupEnvironment();

    char *line;
    size_t buffSize = 32;
    size_t lineSize;
    line = (char *)malloc(buffSize * sizeof(char));    

    int halt = 0;
    while (!halt) {
        printf("Enter your command: ");
        lineSize = getline(&line, &buffSize, (batchMode) ? batchFile : stdin);
        printf("%zu characters were read.\n", lineSize);
        printf("Command: %s", line);
    }
    
    free(line);
    if (batchMode) fclose(batchFile);
    exit(0);
  
  
    
}
