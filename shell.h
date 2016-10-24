/*
 * Date created: 10/18/2016
 * By: Saurav Acharya and Oghenefego Ahia
 * 
 * This file implements the header file for the shell program.
 * 
 */
 
const char *whites = " \t\n";

#define MAX_CHILD_PROCESSES 4096
#define PATH_MAX 4096  /* maximum filepath length in (most) Linux systems */

/* The Environment struct contains variables that need to be passed around
 * while running the shell. */
typedef struct Environment {
    char shell[PATH_MAX];
    char PWD[PATH_MAX];
    pid_t child_processes[MAX_CHILD_PROCESSES];
    int num_child_processes;
} Environment;



