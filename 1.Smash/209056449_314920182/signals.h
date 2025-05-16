#ifndef SIGNALS_H
#define SIGNALS_H

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "commands.h"
#include "jobs.h"

/*=============================================================================
* global Variables
=============================================================================*/

#define CMD_LENGTH_MAX 120

extern pid_t fg_pid;           // current foreground PID
extern char fg_cmd_line[CMD_LENGTH_MAX];   // copy of the command
extern time_t fg_start_time;        // for elapsed time

/*=============================================================================
* global functions
=============================================================================*/
void ctrl_c_handler();
void ctrl_z_handler();
int find_foreground_job();

#endif //__SIGNALS_H__