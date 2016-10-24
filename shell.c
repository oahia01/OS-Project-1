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
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include "cmds.c"

//TODO: fix case where you have to CTRL+D twice on interactive mode

const char *whites = " \t\n";
static volatile int halt = 0;

static Environment *env;

// void sigintHandler(int sig) { printf("what"); }
int setupEnvironment(Environment *env);
void cleanupString(char *line);
void updateRunningChildPids(sem_t *sem_env_ptr);


int main(int argc, char *argv[], char** main_envp) {

    // signal(SIGHUP, sigintHandler);
    
    env = mmap(NULL, sizeof(Environment), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    /* set up shell Environment: */
    if (!setupEnvironment(env)) {
        fprintf(stderr, "Cannot get directory of shell executable.\n");
        exit(1);
    }

    /* set up Environment lock: */
    sem_t sem_env;
    if (sem_init(&sem_env, 1, 1) < 0) {
        fprintf(stderr, "Error initializing semaphore for environment struct.\n");
        exit(1);
    }
        
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

    char *line;
    size_t buffSize = 100;
    size_t lineLimit;
    line = (char *)malloc(buffSize * sizeof(char));    

    while (!halt) {
        // fprintf(stderr, "numRunning: %d\n", env->num_child_processes);

        if (!batchMode){
            printf("%s > ", env->PWD);
        }
        lineLimit = getline(&line, &buffSize, (batchMode) ? batchFile : stdin);
        if (batchMode) printf(line);
        
        /* Check for EOF: */
        if (feof((batchMode) ? batchFile : stdin)) {
            if (!batchMode) printf("\n");
            break;
        }
        
        cleanupString(line);

        /* Handle empty case */
        if (!strcmp(line, "")) continue;

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
            
            /* Handle empty case */
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
            
            runCommand(argv, argc, env, main_envp, &sem_env);
            free(argv);
        }
        free(cmds);
    }
    
    free(line);
    if (batchMode) fclose(batchFile);
    
    /* wait for child processes to close: */
    int i;
    for (i = 0; i < env->num_child_processes; i++) {
        waitpid(env->child_processes[i], NULL, WSTOPPED);
    }
    sem_destroy(&sem_env);
    munmap(env, sizeof(Environment));
    exit(0);
    
}


int setupEnvironment(Environment *env) {
    
    char envbuf[PATH_MAX];
    ssize_t lenBuf;

    if ((lenBuf = readlink("/proc/self/exe", envbuf, sizeof(envbuf) - 1)) == -1)
        return 0;
    else {
        /* copy env->shell and the program environment's SHELL field: */
        // env->shell = malloc(PATH_MAX * sizeof(char*));
        // env->PWD = malloc(PATH_MAX * sizeof(char*));
        strcpy(env->shell, envbuf);
        char var[PATH_MAX+6];
        strcpy(var, "SHELL=");
        strcat(var, env->shell);
        putenv(var);
        
        getcwd(env->PWD, PATH_MAX);
        
        // pid_t cpArr[MAX_CHILD_PROCESSES];
        // env->child_processes = cpArr;
        env->num_child_processes = 0;
        return 1;
    }
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
