#ifndef SIGNALS_H
#define SIGNALS_H

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include "commands.h"
#include "jobs.h"
#define JOB_RUNNUNG_FG 0
#define JOB_RUNNUNG_BG 1
#define JOB_STOPPED 2
#define MAX_JOBS 100
/*=============================================================================
* global functions
=============================================================================*/
void ctrl_c_handler();
void ctrl_z_handler();
int find_foreground_job();

#endif //__SIGNALS_H__