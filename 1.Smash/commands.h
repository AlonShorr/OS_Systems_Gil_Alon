#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>  // for getpid()
#include <string.h>
#include <errno.h>
#include "signal.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include "jobs.h"  // check if this is the correct name alon gave
//#include <sys/types.h>

#define CMD_LENGTH_MAX 120
#define ARGS_NUM_MAX 20
#define MAX_JOBS 100

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

/*=============================================================================
* global functions
=============================================================================*/
int parseCommandExample(char* line);

// requaired functions
int showpid();
int pwd();
int cd(char* path);
int jobs();
int smash_kill(int signum, char* job_id);
int fg(char* job_id);
int bg(char* job_id);
int diff(char* file1, char* file2);
int quit();

// helper functions









#endif //COMMANDS_H