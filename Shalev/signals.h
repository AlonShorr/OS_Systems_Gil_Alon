#ifndef SIGNALS_H
#define SIGNALS_H

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*=============================================================================
* global functions
=============================================================================*/

void ctrl_z_handler(int sig);
void init_signal_handlers();


#endif //__SIGNALS_H__