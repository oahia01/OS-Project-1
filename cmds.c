// TODO: update cd to use env lock as well

#define _GNU_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>

#define MAX_CHILD_PROCESSES 4096

typedef struct Environment {
    char *shell;
    char *PWD;
    pid_t *child_processes;
    int num_child_processes;
} Environment;

void dir(char *path);
void cd(char *path, Environment *env, sem_t *sem_env_ptr);
void environ(char** main_envp);
void echo(char **argv, int argc);
void help();
void pauseShell();
void execute(char** argv, Environment *env);


extern void runCommand(char **argv, int argc, Environment *env, char** main_envp, sem_t *sem_env_ptr) {
    
    // int backgroundRun = !strcmp(argv[argc-1], "&");
    // if (backgroundRun) argc--;
    
    //TODO: handle IO redirection
    
    pid_t child_pid = fork();
    if (child_pid == 0) {
        /* child process: continue running this function */
    } else if (child_pid == -1) {
        fprintf(stderr, "Error creating child process to execute command, errno = %d.\n", errno);
    } else {
        /* parent process: */
        // if (backgroundRun) {
        //     if (env->num_child_processes == MAX_CHILD_PROCESSES) {
        //         fprintf(stderr, "Error: maximum child process limit exceeded. Exiting.\n");
        //         exit(1);
        //     }

        //     sem_wait(sem_env_ptr);
        //     env->child_processes[env->num_child_processes++] = child_pid;
        //     sem_post(sem_env_ptr);
        // } else {
            pid_t w = waitpid(child_pid, NULL, WSTOPPED);
            if (w == -1) fprintf(stderr, "Error waiting for command's child process.\n");
        // }
        return;
    }
    
    /* (for debugging) print the args: */
    // int i;
    // fprintf(stderr, "arg list: ");
    // for (i = 0; i < argc; i++)
    //     fprintf(stderr, "%s - ", argv[i]);
    // fprintf(stderr, "\n");

    char *cmdName = argv[0];
    if (!strcmp(cmdName, "dir")) {
        if      (argc == 1)     dir(env->PWD);
        else if (argc == 2)     dir(argv[1]);
        else    fprintf(stderr, "Error: dir command only takes 0 or 1 argument(s)\n");
    } else if (!strcmp(cmdName, "cd")) {
        if      (argc == 1)     printf("%s\n", env->PWD);
        else if (argc == 2)     cd(argv[1], env, sem_env_ptr);
        else    fprintf(stderr, "Error: cd command only takes 0 or 1 argument(s)\n");
    } else if (!strcmp(cmdName, "clr")) {
        system("clear");
    } else if (!strcmp(cmdName, "environ")) {
        environ(main_envp);
    } else if (!strcmp(cmdName, "echo")) {
        echo(argv, argc);
    } else if (!strcmp(cmdName, "help")) {
        help();
    } else if (!strcmp(cmdName, "pause")) {
        pauseShell();
    } else {
        execute(argv, env);
    }

    exit(0);

}


void dir(char *path) {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "Error opening directory. Check to make sure the directory exists.\n");
    } else {
        /* list files in directory: */
        while ((ent = readdir(dir)) != NULL) {
            printf("%s\n", ent->d_name);
        }
        closedir(dir);
    }
}

void cd(char *path, Environment *env, sem_t *sem_env_ptr) {
    if (chdir(path) != 0) {
        fprintf(stderr, "Error changing directory: ");
        if (errno == 2)
            fprintf(stderr, "make sure new path is valid.\n");
        else
            fprintf(stderr, "errno = %d.\n", errno);
    } else {
        sem_wait(sem_env_ptr);
        getcwd(env->PWD, PATH_MAX);
        
        char var[PATH_MAX+6];
        strcpy(var, "PWD=");
        strcat(var, env->PWD);
        int ret = putenv(var);
        sem_post(sem_env_ptr);
    }
}

void environ(char** main_envp) {
    char **envStr;
    printf("\n");
    for (envStr = main_envp; *envStr != 0; envStr++){
        printf("%s\n",*envStr);
    }
    printf("\n");
}

void echo(char **argv, int argc) {
    int i;
    for (i = 1; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");
}

void help() {
    char *helpStr = "\ncd <directory>\nChange the current default directory to <directory>. If the <directory> argument is not present, reports the current directory.\n\n" 
                      "clr\nClear the screen.\n\n"
                      "dir <directory>\nList the contents of directory <directory>.\n\n"
                      "environ\nList all the environment strings.\n\n"
                      "echo <comment>\nDisplay <comment> on the display followed by a new line (multiple white spaces may be reduced to a single space).\n"
                      "help\nDisplay the user manual using the more filter.\n\n"
                      "pause\nPause operation of the shell until 'Enter' is pressed.\n\n"
                      "quit\nQuit the shell.\n";
    // system(helpStr);
    printf("%s\n", helpStr);
}

void pauseShell() {
    printf("Press 'Enter' to continue...\n");
    while (getchar() != '\n') {}
}

void execute(char** argv, Environment *env) {
    
    // char *var[PATH_MAX+7];
    // strcpy(var, "parent=");
    // strcat(var, env->shell);
    // char *child_env[] = {var, NULL};
    
    // int ret = execvpe(argv[0], argv, child_env);
    // if (ret == -1) {
    //     fprintf(stderr, "Error invoking program: \n");
    //     if (errno == 2) //TODO: handle file not found error
    //         // fprintf(stderr, "Make sure new path is valid.\n");
    //     else
    //         fprintf(stderr, "errno = %d.\n", errno);
    //     exit(1);
    // }
    
}