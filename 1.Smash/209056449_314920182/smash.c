//smash.c

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
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

#define LOGIC_ERROR 2
#define ERROR -1

/*=============================================================================
* classes/structs declarations
=============================================================================*/



/*=============================================================================
* global variables & data structures
=============================================================================*/

char _line[CMD_LENGTH_MAX];
char _cmd[CMD_LENGTH_MAX];

/**
 * @brief global variable to hold the current foreground process ID and command line.
 */
pid_t fg_pid;           // current foreground PID
char fg_cmd_line[CMD_LENGTH_MAX];   // copy of the command
time_t fg_start_time;        // for elapsed time
int signal_flag = 0;

/*=============================================================================
* Functions
=============================================================================*/

/**
 * @brief signal handler for SIGINT (Ctrl+C) and SIGSTP (Ctrl+Z)
 */
void init_signals() {
    struct sigaction ctrlC;
    struct sigaction ctrlZ;

	//ctrlC handler:
    ctrlC.sa_handler = ctrl_c_handler;
    sigemptyset(&ctrlC.sa_mask);
    ctrlC.sa_flags = 0;
    sigaction(SIGINT, &ctrlC, NULL);

	//ctrlZ handler:
    ctrlZ.sa_handler = ctrl_z_handler;
    sigemptyset(&ctrlZ.sa_mask);
    ctrlZ.sa_flags = 0;
    sigaction(SIGTSTP, &ctrlZ, NULL);

	return;
}

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
	for (i = 0; i < ARGS_NUM_MAX && tok; ++i) {
		out->args[i] = tok;
		tok = strtok(NULL, delims);
	}
	out->args[i] = NULL;  // NULL-terminate for execv
	out->nargs    = i;
	out->bg       = 0;

	// check for trailing '&'
	if (out->nargs > 0 && strcmp(out->args[out->nargs - 1], "&") == 0) {
		out->bg = 1;
        out->args[out->nargs - 1] = NULL;
        out->nargs--;
    }
    return 0; // success
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
    if (strcmp(cmd->args[0], "showpid") == 0) {
        if(cmd->nargs != 1) {
			fprintf(stderr, "smash error: showpid: expected 0 arguments\n");
			return 1;
		}
		return showpid();

    } 
	else if (strcmp(cmd->args[0], "pwd") == 0) {
		if(cmd->nargs != 1) {
			fprintf(stderr, "smash error: pwd: expected 0 arguments\n");
			return 1;
		}
        return pwd();

    } 
	else if (strcmp(cmd->args[0], "cd") == 0) {
        if (cmd->nargs != 2) {
            fprintf(stderr, "smash error: cd: expected 1 arguments\n");
            return 1;
        }
        return cd(cmd->args[1]);

    } 
	else if (strcmp(cmd->args[0], "jobs") == 0) {
		if(cmd->nargs != 1) {
			fprintf(stderr, "smash error: jobs: expected 0 arguments\n");
			return 1;
		}
        return s_jobs();

    } 
	else if (strcmp(cmd->args[0], "kill") == 0) {
        if (cmd->nargs != 3) {
            fprintf(stderr, "smash error: kill: invalid arguments\n");
            return 1;
        }
        int signum = atoi(cmd->args[1]);
        char* job_pid = cmd->args[2];
        return smash_kill(signum, job_pid); 

    } 
	else if (strcmp(cmd->args[0], "fg") == 0) {
        if (cmd->nargs > 2) {
            fprintf(stderr, "smash error: fg: invalid arguments\n");
            return 1;
        }
        else if(cmd->nargs == 2)	
			return fg(cmd->args[1]);
		else if(cmd->nargs == 1)
			return empty_fg(); 

	} 
	else if (strcmp(cmd->args[0], "bg") == 0) {
		if (cmd->nargs > 2) {
			fprintf(stderr, "smash error: bg: invalid arguments\n");
			return 1;
		}
		else if(cmd->nargs == 2)	
			return bg(cmd->args[1]);
		else if(cmd->nargs == 1)
			return empty_bg(); 
    } 
	else if (strcmp(cmd->args[0], "diff") == 0) {
        if (cmd->nargs != 3) {
            fprintf(stderr, "smash error: diff: expected 2 arguments\n");
            return 1;
        }
        return diff(cmd->args[1], cmd->args[2]);
    } 
	/*else if (strcmp(cmd->args[0], "quit") == 0) {
		
        if (cmd->nargs > 2) {
            fprintf(stderr, "smash error: quit: expected 0 or 1 arguments\n");
            return 1;
        }
		else if(cmd->nargs == 2 && strcmp(cmd->args[1], "kill") != 0) {
            fprintf(stderr, "smash error: quit: unexpected arguments\n");
            return 1;
        }

        return (cmd->nargs == 1) ? quit() : quit_kill(); //quit with or without kill
        
    }*/

    return 10; // not a built-in
}

/**
 * @brief Handles background jobs and foreground jobs. if fg - updates fg parameters and
 * waits for the process to exit. If bg - adds the job to the jobs array and returns.
 * @param cmd: Command struct with original line parsed.
 * @param pid: Process ID of the child process.
 * @return 0 on success, ERROR if cmd is NULL or cmd->args[0] is NULL
 */
int handle_bg (Command *cmd, pid_t pid) {
    if (!cmd || !cmd->args[0]) 
        return ERROR;
    if(!cmd->bg) {
        fg_pid = pid; // set fg pid to the new process
        strcpy(fg_cmd_line, _line);
        fg_start_time = time(NULL);
        int status;
        waitpid(pid, &status, WUNTRACED); // wait for the child to finish
        if (WIFSTOPPED(status)) {
            add_job(pid, _line, JOB_STOPPED);
        }

    }
	else if (cmd->bg) {
        // Only allow background for certain built-ins
        if (strcmp(cmd->args[0], "cd") == 0 || strcmp(cmd->args[0], "fg") == 0 || strcmp(cmd->args[0], "quit") == 0) {
            fprintf(stderr, "smash error: %s cannot run in background\n", cmd->args[0]);
            return 1;
        }
        add_job(pid, _line, JOB_RUNNING_BG); //TODO: check success?
        return 0;
    }
    
    //Update the job status to running in the background from smash
    if(strcmp(cmd->args[0], "bg") == 0) {
        if(cmd->nargs == 1) {
            int max_id = -1;
            for (int i = 0; i < MAX_JOBS; i++) {
                if (get_pid(jobs_arr[i]) != 0) {
                    if (jobs_arr[i].status == JOB_STOPPED) {
                        max_id = i;
                    }
                }
            }
            if (max_id != -1)
                update_job_status(max_id, JOB_RUNNING_BG);
        }   
        else if(cmd->nargs == 2) {
            int job_index = str_to_int(cmd->args[1]);
            update_job_status(job_index, JOB_RUNNING_BG); 
        }
    }

    return 0;
}

/**
 * @brief Launches an external command using execvp.
 * @param cmd: Command struct with original line parsed.
 * @return 0 on success, ERROR if cmd is NULL or cmd->args[0] is NULL
 */
int launch_external(Command *cmd) {
    if (execvp(cmd->args[0], cmd->args) == -1) { 
        exit(EXIT_FAILURE);
        return ERROR; // this line will never be reached, but added for safety 
    }
    return 0; // success
} 

int handle_quit(Command *cmd) {
    if (cmd->nargs > 2) {
        fprintf(stderr, "smash error: quit: expected 0 or 1 arguments\n");
        return 1;
    }
    else if(cmd->nargs == 2 && strcmp(cmd->args[1], "kill") != 0) {
        fprintf(stderr, "smash error: quit: unexpected arguments\n");
        return 1;
    }

    return (cmd->nargs == 2) ? quit_kill() : 0;
}
 /*=============================================================================
  * main function
  *=============================================================================*/
int main(int argc, char* argv[])
{
	Command cmd;
	init_signals(); // sets signal handlers from default to our custom ones (so bash won't kill smash :( ) 
    init_jobs(); // Initialize the jobs array
    while (1) {
        int father_command = 0;
        fg_pid = getpid(); //initialize fg_pid to smash pid

        // 1) print prompt and flush stdout
        printf("smash > ");
        fflush(stdout);

        // 2) read a line of input
        char* check = fgets(_line, CMD_LENGTH_MAX, stdin);
        // signal handling: if Ctrl+C or Ctrl+Z is pressed, we set the signal_flag to 1
        // and continue to the next iteration of the loop.
        if (check == NULL) {
			 // EOF or error
             if (signal_flag) {
                signal_flag = 0; // Reset the flag
                continue;
            } 
            else{
                break;
            }
		 }
         
         _line[strcspn(_line, "\n")] = '\0';

		 // copy raw input if needed elsewhere
		 strcpy(_cmd, _line);

        // 3) parse into Command struct
        if (parseCommand(_cmd, &cmd) < 0) {
			 // empty line — loop again
			 _line[0] = '\0';
			 _cmd[0]  = '\0';
			 continue;
		 }
 
        //4) check for quit or father commands (commands that should be run by smash)
        update_jobs(); 
        if (strcmp(cmd.args[0], "quit") == 0){
            int check = handle_quit(&cmd); // quit command
            if (check == 1) {
                continue;
            }
            else {
                break;
            } 
        }
        if (strcmp(cmd.args[0], "showpid") == 0
            || strcmp(cmd.args[0], "pwd") == 0
            || strcmp(cmd.args[0], "cd") == 0
            || strcmp(cmd.args[0], "jobs") == 0
            || strcmp(cmd.args[0], "fg") == 0){
            father_command = 1;
            handle_builtin(&cmd);
        }

        // 5) execute/dispatch
        update_jobs(); 

        if(!father_command) {
            pid_t pid = fork(); // Fork a new process
            if (pid < 0)
            {
                perror("smash error: fork failed");
                return LOGIC_ERROR;
            }
            else if (pid == 0) { // Child process
                setpgrp(); // Set the child to a new process group
                int is_internal = handle_builtin(&cmd); // check if the command is a built-in command
                if(is_internal == 10) { // not a built-in command
                    launch_external(&cmd); // launch the external command
                } 
                exit(0); // Exit child process after executing the command
            }
            else if (pid != 0) {
                handle_bg(&cmd, pid); // handle jobs array
            }
            else {
                fprintf(stderr, "smash error: fork failed\n");
                return LOGIC_ERROR;
            }
        }    
        // 6) update job list
        update_jobs(); 
 
		 // clear buffers for next iteration
		 _line[0] = '\0';
		 _cmd[0]  = '\0';
	}
    
    return 0;
}






