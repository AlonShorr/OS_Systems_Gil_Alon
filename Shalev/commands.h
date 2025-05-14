#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define CMD_LENGTH_MAX 120
#define ARGS_NUM_MAX 20
#define JOBS_NUM_MAX 100

/*=============================================================================
* error handling - some useful macros and examples of error handling,
* feel free to not use any of this
=============================================================================*/
typedef struct CmdArgs_t {
	bool bg;
	char* args[ARGS_NUM_MAX];
	int nargs;
	char* cmd;
	char* last_cmd;
	} CmdArgs_t;

CmdArgs_t* initCmdArgs();
void freeCmdArgs(CmdArgs_t* cmdArgs);



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
CmdArgs_t* parseCmd(char* line);

#endif //COMMANDS_H