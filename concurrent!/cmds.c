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
#include <pthread.h>

#define MAX_THREADS 4096

typedef struct Environment {
    char *shell;
    char *PWD;
    pthread_t *threads;
    int num_threads;
} Environment;

typedef struct RunParameters {
    char **argv;
    int argc;
    Environment *env;
    char** main_envp;
    sem_t *env_lock;
    pthread_t *threadID;
} RunParameters;

extern void runCommand(char **argv, int argc, Environment *env, char** main_envp, sem_t *env_lock);
extern void runCommand2(char **argv, int argc, Environment *env, char** main_envp, sem_t *env_lock);  /* I split runCommand to make debugging easier, join them back later */
void *runCommandConcurrent(void *runParamsVoid);
void removeThread(Environment *env, pthread_t *thread);

void dir(char *path);
void cd(char *path, Environment *env, sem_t *env_lock);
void environ(char** main_envp);
void echo(char **argv, int argc);
void help();
void pauseShell();
void execute(char** argv, Environment *env);

extern void runCommand(char **argv, int argc, Environment *env, char** main_envp, sem_t *env_lock) {
    
    /* handle background (concurrent) run: */
    int backgroundRun = !strcmp(argv[argc-1], "&");
        fprintf(stderr, "zero\n");
    
    if (backgroundRun) {
        sem_wait(env_lock);
        if (env->num_threads == MAX_THREADS) {
            fprintf(stderr, "Error: could not run command concurrently; maximum child process limit exceeded.\n");
            return;
        }
        sem_post(env_lock);

        fprintf(stderr, "one\n");
        /* run command in another thread, and keep thread ID: */
        pthread_t new_thread;
        RunParameters runParams = { .argv = argv, .argc = argc, .env = env, .main_envp = main_envp,
                                    .env_lock = env_lock, .threadID = &new_thread };
        if (pthread_create(&new_thread, NULL, runCommandConcurrent, &runParams)) {
            fprintf(stderr, "Error running command in a separate thread.\n");
            return;
        }

        sem_wait(env_lock);
        env->threads[env->num_threads++] = new_thread;
        sem_post(env_lock);
        fprintf(stderr, "two\n");
        return;
    }

    //TODO: handle IO redirection
    runCommand2(argv, argc, env, main_envp, env_lock);
}

extern void runCommand2(char **argv, int argc, Environment *env, char** main_envp, sem_t *env_lock) {

    /* run the command in a child process: */
    pid_t child_pid = fork();
    if (child_pid == 0) {
        /* child process: continue running this function */
    } else if (child_pid == -1) {
        fprintf(stderr, "Error creating child process to execute command, errno = %d.\n", errno);
        return;
    } else {
        /* parent process: wait for child process to finish */
        pid_t w = waitpid(child_pid, NULL, WSTOPPED);
        fprintf(stderr, "parent process returning\n");
        if (w == -1) fprintf(stderr, "Error waiting for command's child process.\n");
        return;
    }
    
    /* (for debugging) print the args: */
    // int i;
    // fprintf(stderr, "arg list: ");
    // for (i = 0; i < argc; i++)
    //     fprintf(stderr, "%s - ", argv[i]);
    // fprintf(stderr, "\n");

    sem_wait(env_lock);
    char *cmdName = argv[0];
    if (!strcmp(cmdName, "dir")) {
        if      (argc == 1)     dir(env->PWD);
        else if (argc == 2)     dir(argv[1]);
        else    fprintf(stderr, "Error: dir command only takes 0 or 1 argument(s)\n");
    } else if (!strcmp(cmdName, "cd")) {
        if      (argc == 1)     printf("%s\n", env->PWD);
        else if (argc == 2)     cd(argv[1], env, env_lock);
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
    sem_post(env_lock);


    exit(0);

}


void *runCommandConcurrent(void *runParamsVoid) {

    RunParameters *runParams = (RunParameters *)runParamsVoid;

    /* in this thread, run the command without background run (remove the &) */
    fprintf(stderr, "oneConcurrent\n");
    runCommand(runParams->argv, runParams->argc-1, runParams->env, runParams->main_envp, runParams->env_lock);
    
    /* before closing this thread, remove this thread from env->threads */
    sem_wait(runParams->env_lock);
    removeThread(runParams->env, runParams->threadID);
    sem_post(runParams->env_lock);
    return NULL;
}


void removeThread(Environment *env, pthread_t *thread) {
    int i;
    for (i = 0; i < env->num_threads; i++)
        if (pthread_equal(*thread, env->threads[i])) break;
    env->num_threads--;
    while (i < env->num_threads) {
        env->threads[i] = env->threads[i+1];
        i++;
    }
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

void cd(char *path, Environment *env, sem_t *env_lock) {
    fprintf(stderr, "cd envoked\n");
    if (chdir(path) != 0) {
        fprintf(stderr, "Error changing directory: ");
        if (errno == 2)
            fprintf(stderr, "make sure new path is valid.\n");
        else
            fprintf(stderr, "errno = %d.\n", errno);
    } else {
        sem_wait(env_lock);
        getcwd(env->PWD, PATH_MAX);
        
        char var[PATH_MAX+6];
        strcpy(var, "PWD=");
        strcat(var, env->PWD);
        int ret = putenv(var);
        sem_post(env_lock);
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