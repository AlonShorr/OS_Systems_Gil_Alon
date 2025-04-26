#ifndef COMMANDS_H_TEST
#define COMMANDS_H_TEST
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdlib.h>
#include <stdio.h>

int showpid();
int pwd();
int cd(char* path);
int jobs();
int smash_kill(int signum, char* job_id);
int fg(char* job_id);
int bg(char* job_id);
int diff(char* file1, char* file2);
int quit();
void update_jobs(); 

#endif //COMMANDS_H_TEST
