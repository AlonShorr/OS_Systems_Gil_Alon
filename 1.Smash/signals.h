#ifndef SIGNALS_H
#define SIGNALS_H

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include "commands.h"
#include "jobs.h"


/*=============================================================================
* global functions
=============================================================================*/
void ctrl_c_handler();
void ctrl_z_handler();
int find_foreground_job();

#endif //__SIGNALS_H__