//commands.c
#define _POSIX_C_SOURCE 200809L
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

//example function for printing errors from internal commands
void perrorSmash(const char* cmd, const char* msg)
{
    fprintf(stderr, "smash error:%s%s%s\n",
        cmd ? cmd : "",
        cmd ? ": " : "",
        msg);
}

	
CmdArgs_t* initCmdArgs() {
    CmdArgs_t* cmdArgs = (CmdArgs_t*)malloc(sizeof(CmdArgs_t));
    cmdArgs->cmd = (char*)malloc(sizeof(char) * CMD_LENGTH_MAX);
    cmdArgs->last_cmd = (char*)malloc(sizeof(char) * CMD_LENGTH_MAX);
    if (!cmdArgs->last_cmd) {
        free(cmdArgs->cmd);
        free(cmdArgs);
        return NULL;
    }
    if (!cmdArgs->cmd) {
        free(cmdArgs->last_cmd);
        free(cmdArgs);
        return NULL;
    }
    if (!cmdArgs->cmd) {
        free(cmdArgs);
        return NULL;
    }

    if (!cmdArgs) {
        return NULL;
    }
 
    cmdArgs->bg = false;
    cmdArgs->nargs = 0;
    for (int i = 0; i < ARGS_NUM_MAX; i++) {
        cmdArgs->args[i] = NULL;
    }
    
    return cmdArgs;
}

void freeCmdArgs(CmdArgs_t* cmdArgs) {
    if (!cmdArgs) {
        return;
    }
    for (int i = 0; i < ARGS_NUM_MAX; i++) {
        if (cmdArgs->args[i]) {
            free(cmdArgs->args[i]);
        }
    }
    free(cmdArgs);
}

//example function for parsing commands
CmdArgs_t* parseCmd(char* line)
{
    CmdArgs_t* cmdArgs = initCmdArgs();
    if (!cmdArgs)
        return NULL;
    strcpy(cmdArgs->last_cmd, cmdArgs->cmd);
    strcpy(cmdArgs->cmd, line);

    strcpy(cmdArgs->cmd, line);

    size_t len = strlen(cmdArgs->cmd);
    while (len > 0 && isspace((unsigned char)cmdArgs->cmd[len - 1])) {
          cmdArgs->cmd[--len] = '\0';
        }

    if (len > 0 && cmdArgs->cmd[len - 1] == '&') {
        cmdArgs->cmd[--len] = '\0';

    while (len > 0 && isspace((unsigned char)cmdArgs->cmd[len - 1])) {
        cmdArgs->cmd[--len] = '\0';
    }
        }


    char* delimiters = " \t\n";
    char* token = strtok(line, delimiters);
    if (!token)
        return NULL;

    int nargs = 0;

    // First token is the command name
    cmdArgs->args[0] = (char*)malloc(strlen(token) + 1);
    if (!cmdArgs->args[0]) {
        free(cmdArgs);
        return NULL;
    }
    strcpy(cmdArgs->args[0], token);
    nargs++;

    // Parse remaining arguments
    for (int i = 1; i < ARGS_NUM_MAX; i++) {
        token = strtok(NULL, delimiters);
        if (!token)
            break;

        cmdArgs->args[i] = (char*)malloc(strlen(token) + 1);
        if (!cmdArgs->args[i]) {
            // Clean up in case of malloc failure
            for (int j = 0; j < i; j++) {
                free(cmdArgs->args[j]);
            }
            free(cmdArgs);
            return NULL;
        }
        strcpy(cmdArgs->args[i], token);
        nargs++;
    }

    // Check if last argument is "&"
    if (nargs > 0 && strcmp(cmdArgs->args[nargs - 1], "&") == 0) {
        free(cmdArgs->args[nargs - 1]); // free the "&"
        cmdArgs->args[nargs - 1] = NULL;
        cmdArgs->bg = true;
        nargs--;
        // if (nargs > 0) {
		// 	cmdArgs->args[nargs - 1][strlen(cmdArgs->args[nargs - 1])] = '\0';
		// }
    } else {
        cmdArgs->bg = false;
    }

    cmdArgs->nargs = nargs;
    
 
    return cmdArgs;
}


