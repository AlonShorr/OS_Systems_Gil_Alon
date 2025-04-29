//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "commands.h"
#include "signals.h"
#include "jobs.h"

#define CMD_LENGTH_MAX  80
#define MAX_ARGS        20
#define ERROR -1

/*=============================================================================
* classes/structs declarations
=============================================================================*/

// Command structure: argv-style args array + bg flag
typedef struct {
	char* args[MAX_ARGS + 1];  // NULL-terminated argument list (first argument == command name)
	int   nargs;               // number of arguments (excluding NULL)
	int   bg;                  // non-zero if command ends with '&'
} Command;

/*=============================================================================
* global variables & data structures
=============================================================================*/

char _line[CMD_LENGTH_MAX];
char _cmd[CMD_LENGTH_MAX];

/*=============================================================================
* Functions
=============================================================================*/

/**
 * @brief tokenize input line into the Command struct.
 * @param cmd_input:  input buffer (will be modified by strtok) that the user entered to the cmd
 * @param out:        pointer to Command struct to populate
 * @return 0 on success, ERROR if line is empty
 */
int parseCommand(char *cmd_input, Command *out) {
	const char *delims = " \t\n";
	char *tok;
	int   i = 0;

	// first token = command name
	tok = strtok(cmd_input, delims);
	if (!tok)
		return ERROR;   // no command entered

	// collect up to MAX_ARGS tokens
	for (i = 0; i < MAX_ARGS && tok; ++i) {
		out->args[i] = tok;
		tok = strtok(NULL, delims);
	}
	out->args[i] = NULL;  // NULL-terminate for execv
	out->nargs    = i;
	out->bg       = 0;

	// check for trailing '&'
	if (out->nargs > 0 && strcmp(out->args[out->nargs - 1], "&") == 0) 
	{
		out->bg = 1;
		out->args[out->nargs - 1] = NULL;
		out->nargs--;
	}
	return 0;
}


/**
 * @brief Built‑In Command Dispatcher
 * Checks cmd->argv[0] against built‑ins
 * Validates argument counts and prints errors
 * knows to fork cmd to smash child process if '&' is given
 * @param cmd: cmd struct with original line parsed
 * @return function call on success, 0 if not built-in, 1 for nargs error,
 * ERROR if cmd name == NULL
 * and the the function call if works correctly.
 */
int handle_builtin(Command *cmd) {
    if (!cmd || !cmd->args[0])
        return ERROR;

	  // Background built-in handling
	  if (cmd->bg) {
        // Only allow background for certain built-ins
        if (strcmp(cmd->args[0], "cd") == 0 || strcmp(cmd->args[0], "fg") == 0 || strcmp(cmd->args[0], "quit") == 0) {
            fprintf(stderr, "smash error: %s cannot run in background", cmd->args[0]);
            return 1;
        }

        int pid = fork();
        if (pid < 0) {
            perror("smash error: fork failed");
            return 1;
        }
        if (pid == 0) {
            setpgrp(); //sets child to a new process group so ctrl C/Z wont affect smash main process
            cmd->bg = 0; // Run as foreground inside the child
            handle_builtin(cmd);
            exit(0);
        } else {
            add_job(pid, cmd->args[0], JOB_RUNNING_BG); //TODO: check success?
            return 1;
        }
    }

    if (strcmp(cmd->args[0], "showpid") == 0) {
        if(cmd->nargs != 1) {
			fprintf(stderr, "smash error: showpid: expected 0 arguments\n");
			return 1;
		}
		return showpid();

    } else if (strcmp(cmd->args[0], "pwd") == 0) {
		if(cmd->nargs != 1) {
			fprintf(stderr, "smash error: pwd: expected 0 arguments\n");
			return 1;
		}
        return pwd();

    } else if (strcmp(cmd->args[0], "cd") == 0) {
        if (cmd->nargs != 2) {
            fprintf(stderr, "smash error: cd: expected 1 arguments\n");
            return 1;
        }
        return cd(cmd->args[1]);

    } else if (strcmp(cmd->args[0], "jobs") == 0) {
		if(cmd->nargs != 1) {
			fprintf(stderr, "smash error: jobs: expected 0 arguments\n");
			return 1;
		}
        return jobs();

    } else if (strcmp(cmd->args[0], "kill") == 0) {
        if (cmd->nargs != 3) {
            fprintf(stderr, "smash error: kill: invalid arguments\n");
            return 1;
        }
        int signum = atoi(cmd->args[1]);
        int job_pid = atoi(cmd->args[2]);
        return kill(signum, job_pid); //TODO: tell gil to change it to smash kill for conflict solving

    } else if (strcmp(cmd->args[0], "fg") == 0) {
        if (cmd->nargs != 2) {
            fprintf(stderr, "smash error: fg: invalid arguments\n");
            return 1;
        }
        return fg(cmd->args[1]);

    } else if (strcmp(cmd->args[0], "bg") == 0) {
        if (cmd->nargs != 2) {
            fprintf(stderr, "smash error: bg: invalid arguments\n");
            return 1;
        }
        return bg(cmd->args[1]);

    } else if (strcmp(cmd->args[0], "diff") == 0) {
        if (cmd->nargs != 3) {
            fprintf(stderr, "smash error: diff: expected 2 arguments\n");
            return 1;
        }
        return diff(cmd->args[1], cmd->args[2]);

    } else if (strcmp(cmd->args[0], "quit") == 0) {
		if (cmd->nargs > 2 || cmd->nargs == 2) {
            fprintf(stderr, "smash error: quit: expected 0 or 1 arguments\n");
            return 1;
        }

		else if(strcmp(cmd->args[1], "kill") != 0) {
            fprintf(stderr, "smash error: quit: unexpected arguments\n");
            return 1;
        }

        return quit();
    }

    return 0; // not a built-in
}

/**
 * @brief forks external cmds to a child process. if bg than it immidetly moves it
 * to the jobs queue as JOB_RUNNING_BG. else waits for it to finish or if stopped moves
 * it to the jobs queue as JOB_STOPPED
 * @param cmd: the cmd struct after parsing.
 */
void launch_external(Command *cmd) { 
		int pid = fork();
		if (pid < 0) {
            perror("smash error: fork failed");
            return;
        }
		else if(pid == 0) {
			setpgrp();
    		execvp(cmd->args[0], cmd->args);
    		perror("smash error: execvp failed");
    		exit(1);
		}
		else {
			int status;
			if(!cmd->bg) {
				waitpid(pid, &status, WUNTRACED);
				if (WIFSTOPPED(status)) { //macro to indicate if the child was stopped
					add_job(pid, cmd->args[0], JOB_STOPPED); 
					//printf("smash: process %d was stopped\n", pid);
				}	//if the process exited or killed to alteration to  jobs required.
            }
			else if(cmd->bg) {
			add_job(pid, cmd->args[0], JOB_RUNNING_BG);
			//printf("smash: background process %d started (%s)\n", pid, cmd->args[0]); //TODO: any print required?
			} 
		}
}


 /*=============================================================================
  * main function
  *=============================================================================*/
//TODO: take care of ctrl C and ctrl Z!!!!!!!!!!!!!! calling ctrl C and Z handler
// also make sure smash prints  prompt after  ctrl C and  Z
int main(int argc, char* argv[])
 {
	 Command cmd;
 
	 /*sets signal handlers from default to our custom ones (so bash won't kill smash :( )*/
	init_signals();    // to be implemented
 
	 while (1) {
		 // 1) print prompt
		 printf("smash > ");
		 fflush(stdout);
 
		 // 2) read a line of input
		 if (!fgets(_line, CMD_LENGTH_MAX, stdin)) {
			 // EOF or error
			 break;
		 }
 
		 // copy raw input if needed elsewhere
		 strcpy(_cmd, _line);
 
		 // 3) parse into Command struct
		 if (parseCommand(_cmd, &cmd) < 0) {
			 // empty line — loop again
			 _line[0] = '\0';
			 _cmd[0]  = '\0';
			 continue;
		 }
 
		 //4) execute/dispatch 
		 if (!handle_builtin(&cmd)) 
			launch_external(&cmd); //Forks and runs external programs (e.g. ls, sleep)

		 // 5) update job list, reap finished, etc. (to implement)
		 update_jobs(); 
 
		 // clear buffers for next iteration
		 _line[0] = '\0';
		 _cmd[0]  = '\0';
	 }
 
	 return 0;
 }






