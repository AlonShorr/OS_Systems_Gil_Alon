#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h> 
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "signals.h"
#include "jobs.h"
#include <sys/types.h>

#define CMD_LENGTH_MAX 120
#define ARGS_NUM_MAX 20
#define ERROR -1
#define MAX_JOBS 100

//#define JOB_RUNNING_FG 2
#define JOB_RUNNING_BG 1
#define JOB_STOPPED 2
#define PATH_MAX 4096

#define SMASH_SUCCESS 0
#define SMASH_FAIL 1
#define SMASH_ERROR 2
#define SMASH_QUIT 3 



extern job jobs_arr[MAX_JOBS]; // array of jobs
extern int jobs_num;
extern pid_t fg_pid;           // current foreground PID
extern char fg_cmd_line[CMD_LENGTH_MAX];   // copy of the command
extern time_t fg_start_time;        // for elapsed time


// Command structure: argv-style args array + bg flag
typedef struct {
	char* args[ARGS_NUM_MAX + 1];  // NULL-terminated argument list (first argument == command name)
	int   nargs;               // number of arguments (excluding NULL)
	int   bg;                  // non-zero if command ends with '&'
}Command;

/*=============================================================================
* error handling - some useful macros and examples of error handling,
* feel free to not use any of this
=============================================================================*/
#define ERROR_EXIT(msg) \
    do { \
        fprintf(stderr, "%s: %d\n%s", __FILE__, __LINE__, msg); \
        exit(1); \
    } while(0);

static inline void* _validatedMalloc(size_t size)
{
    void* ptr = malloc(size);
    if(!ptr) ERROR_EXIT("malloc");
    return ptr;
}

// example usage:
// char* bufffer = MALLOC_VALIDATED(char, MAX_LINE_SIZE);
// which automatically includes error handling
#define MALLOC_VALIDATED(type, size) \
    ((type*)_validatedMalloc((size)))


/*=============================================================================
* error definitions
=============================================================================*/
/*
typedef enum  {
	INVALID_COMMAND = 0,
	//feel free to add more values here or delete this
} ParsingError;

typedef enum {
	SMASH_SUCCESS = 0,
	SMASH_QUIT,
	SMASH_FAIL
	//feel free to add more values here or delete this
} CommandResult;
*/
/*=============================================================================
* global functions
=============================================================================*/
int parseCommandExample(char* line);

// requaired functions
int showpid();
int pwd();
int cd(char* path);
int s_jobs();
int smash_kill(int signum, char* job_id); //perhaps parent
int fg(char* job_id);
int empty_fg();
int bg(char* job_id);
int empty_bg();
int diff(char* file1, char* file2); //perhaps parent
int quit();
int quit_kill();

// helper functions

int str_to_int(char* str);








#endif //COMMANDS_H