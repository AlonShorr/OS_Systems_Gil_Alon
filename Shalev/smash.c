//smash.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "commands.h"
#include "signals.h"
#include <ctype.h>
#include "stdbool.h"
#include <unistd.h>   // for access()
#include <dirent.h>   // for opendir()
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include "signals.h"
#include <sys/types.h>




jmp_buf env_buf; // global variable to store the environment for longjmp

typedef enum {
	FILE_TYPE,	//0
	DIR_TYPE,	//1
	NOT_FOUND	//2
} path_type;

typedef struct job_t job_t;
CmdArgs_t* cmdArgs=NULL;\
char cmd_last_command[CMD_LENGTH_MAX];
/*=============================================================================
* classes/structs declarations
=============================================================================*/
struct job_t {
	int job_id;
	int pid;
	char* cmd;
	bool is_stopped;
	int time_elapsed;
	bool is_bg;
};

typedef struct job_array_t {
	job_t** jobs;
	int size;
} job_array_t;


typedef struct smash_t {
	job_array_t* job_array;
	char* current_dir;
	char* last_dir;
	int fg_pid;
} smash_t;
smash_t* smash=NULL; //global struct


void check_finished_jobs();

job_array_t* create_job_array(){
	job_array_t* job_array = (job_array_t*)malloc(sizeof(job_array_t));
	job_array->jobs = (job_t**)malloc(sizeof(job_t*) * JOBS_NUM_MAX);
	job_array->size = 0;
	return job_array;
}

void delete_job_array(job_array_t* job_array){
	free(job_array->jobs);
	free(job_array);
}

void init_smash(){
	smash = (smash_t*)malloc(sizeof(smash_t));
	smash->job_array = create_job_array();
	smash->current_dir = getcwd(NULL, 0);
	smash->last_dir = NULL;
	smash->fg_pid = 0;
	 init_signal_handlers();  // <-- this sets up Ctrl+Z handling

}

void delete_smash()	
{
	delete_job_array(smash->job_array);
	free(smash->current_dir);
	free(smash->last_dir);
	free(smash);
}


job_t* create_job( int pid, char* cmd, bool is_stopped, int time_elapsed, bool is_bg) {
	if(strlen(cmd) > CMD_LENGTH_MAX+1) {
		printf("ERROR SHALEV AMRA - create_job"); //add really error
		return NULL;
	}
	job_t* job = (job_t*)malloc(sizeof(job_t));
	if(job == NULL) {
		printf("Failed to allocate memory for job\n"); //TODO: really error
		return NULL;
	}

	//job->pid = getpid();
	job->pid = pid;
	char* temp = (char*)malloc(sizeof(char) * CMD_LENGTH_MAX);
	if(temp == NULL) {
		printf("Failed to allocate memory for job\n"); //TODO: really error
		free(job);
		return NULL;
	}

	strcpy(temp, cmd);
	job->cmd = temp;
	job->is_stopped = is_stopped;
	job->time_elapsed = time(NULL);
	job->is_bg = is_bg;
	return job;
}
void delete_job(job_t* job){
	free(job->cmd);
	free(job);
}

void insert_job(job_array_t* job_array, job_t* job) {
	if(job_array->size >= JOBS_NUM_MAX) {
		printf("smash error: too many jobs\n");
		return;
	}
	if(job_array == NULL || job == NULL) {
		printf("smash error: insert_job: invalid arguments\n");
		return;
	}

	for(int i = 0; i <JOBS_NUM_MAX; i++) {
		if(job_array->jobs[i] == NULL) {
			job_array->jobs[i] = job;
			job->job_id = i;
			break;
		}
	}
	job_array->size++;
}

void remove_job(job_array_t* job_array, job_t* job) {
	if(job_array == NULL || job == NULL) {
		printf("smash error: remove_job: invalid arguments\n");
		return;
	}
	if(job_array->jobs[job->job_id]==job){
		job_array->jobs[job->job_id] = NULL;
		delete_job(job);
		job_array->size--;
		return;
	}
	else {
		printf("smash error: remove_job: job not found\n");
		return;
	}
}


double get_time_elapsed(job_t* job) {
	if(job == NULL) {
		printf("smash error: get_time_elapsed: invalid arguments\n");
		return -1;
	}
	return difftime(time(NULL), job->time_elapsed);
}

int print_all_jobs(job_array_t* job_array) {	
	if(job_array == NULL) {
		printf("smash error: print_all_jobs: invalid arguments\n");
		return 1;
	}	
	for(int i = 0; i < JOBS_NUM_MAX; i++) {
		if(job_array->jobs[i] != NULL) {
			if(job_array->jobs[i]->is_bg)
				printf("[%d] %s &: %d %d secs %s\n", job_array->jobs[i]->job_id, job_array->jobs[i]->cmd,
					job_array->jobs[i]->pid, (int)get_time_elapsed(job_array->jobs[i]),
					job_array->jobs[i]->is_stopped ? "(stopped)" : "");

			else 	
				printf("[%d] %s: %d %d secs %s\n", job_array->jobs[i]->job_id, job_array->jobs[i]->cmd,
			    	job_array->jobs[i]->pid, (int)get_time_elapsed(job_array->jobs[i]),
			    	job_array->jobs[i]->is_stopped ? "(stopped)" : "");
		
		}	
	}
	return 0;
}

int jobs(CmdArgs_t* cmdArgs) {
	if (cmdArgs == NULL) {
		printf("smash error: jobs: invalid arguments\n");
		return 1;
	}

	int argc = cmdArgs->nargs;
	check_finished_jobs();
	if(argc == 1 ) { //maybe add || (argc==2 && cmdArgs->bg ==true)
		int status = print_all_jobs(smash->job_array);
		return status;
	}
	else{
		printf("smash error: jobs: expected 0 arguments\n");
		return 1;
	}
	return 0;
}

int showpid(CmdArgs_t* cmdArgs) {
	if(cmdArgs == NULL) {
		printf("smash error: showpid: invalid arguments\n");
		return 1;
	}
	int argc = cmdArgs->nargs;
	if(smash->fg_pid != 0) {
		printf("smash pid is %d\n", getppid());
		return 0;
	}
	if(argc == 1) {
		printf("smash pid is %d\n", getpid());
		return 0;
	}
	else {
		printf("smash error: showpid: expected 0 arguments\n");
		return 1;
	}
	return 0;
}

int pwd(CmdArgs_t* cmdArgs) {
	if(cmdArgs == NULL) {
		printf("smash error: pwd: invalid arguments\n");
		return 1;
	}

	int argc = cmdArgs->nargs;
	if(argc == 1) {
		printf("%s\n", smash->current_dir);
		return 0;
	}
	else {
		printf("smash error: pwd: expected 0 arguments\n");
		return 1;
	}
	return 0;
}

int check_path_type(const char* path) {
    DIR* dir = opendir(path);
    if (dir != NULL) {
        closedir(dir);
        return DIR_TYPE;
    }

    FILE* file = fopen(path, "r");
    if (file != NULL) {
        fclose(file);
        return FILE_TYPE;
    }

    return NOT_FOUND;
}

int cd(CmdArgs_t* cmdArgs) {
	if(cmdArgs == NULL){
		return 1;
	}

    if (cmdArgs->nargs != 2) {
        printf("smash error: cd: expected 1 argument\n");
        return 1;
    }

    char* target = cmdArgs->args[1];

    // Handle "cd -"
    if (strcmp(target, "-") == 0) {
        if (!smash->last_dir) {
            printf("smash error: cd: old pwd not set\n");
            return 1;
        }
        if (chdir(smash->last_dir) != 0) {
            perror("smash error: cd");
            return 1;
        }
        printf("%s\n", smash->last_dir);

        char* new_current = getcwd(NULL, 0);
        if (!new_current) {
            perror("getcwd");
            return 1;
        }

        free(smash->last_dir);
        smash->last_dir = smash->current_dir;
        smash->current_dir = new_current;
        return 0;
    }

    // Attempt to change directory
    if (chdir(target) == 0) {
        // Success — update dirs
        char* new_current = getcwd(NULL, 0);
        if (!new_current) {
            perror("getcwd");
            return 1;
        }

        free(smash->last_dir);
        smash->last_dir = smash->current_dir;
        smash->current_dir = new_current;
        return 0;
    }

    // If chdir failed — check why
    int type = check_path_type(target);
    if (type == FILE_TYPE) {
        printf("smash error: cd: %s: not a directory\n", target);
    } else if (type == NOT_FOUND) {
        printf("smash error: cd: target directory does not exist\n");
    } else {
        perror("smash error: cd");
    }

    return 1;
}




enum {
	VALID_DIGIT,
	WITH_LETTER,
	INVALID_DIGIT
};

int is_valid_number(const char *str) {
    if (*str == '\0') return WITH_LETTER;  // empty string

    // Check that all characters are digits
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i]))
            return WITH_LETTER;
    }

    // Convert to int and check range
    int value = atoi(str);
    return value >= 0 && value <= 100 ? VALID_DIGIT : INVALID_DIGIT;
}


int bg(CmdArgs_t* cmdArgs) {	
	if(cmdArgs == NULL) {
		printf("smash error: bg: invalid arguments\n");
		return 1;
	}
	int argc = cmdArgs->nargs;

	if(argc == 1) {
		for(int i = JOBS_NUM_MAX - 1; i >= 0; i--) {
			if(smash->job_array->jobs[i] != NULL) {
				if(smash->job_array->jobs[i]->is_stopped) {
					printf("%s: %d\n", smash->job_array->jobs[i]->cmd, smash->job_array->jobs[i]->pid);
					kill(smash->job_array->jobs[i]->pid, SIGCONT);
					smash->job_array->jobs[i]->is_stopped = false;
					return 0;
				}
			}
			if(i==0) {
				printf("smash error: bg: there are no stopped jobs to resume\n");
				return 1;
			}
		}
	}

	if(argc > 2) {
		printf("smash error: bg: invalid arguments\n");
		return 1;
	}


	int status_number = is_valid_number(cmdArgs->args[1]);
	if(status_number == WITH_LETTER) {
		printf("smash error: bg: invalid arguments\n");
		return 1;
	}
	else if(status_number == INVALID_DIGIT) {
		printf("smash error: bg: job id %s does not exist\n", cmdArgs->args[1]);
		return 1;
	}

	int job_id = atoi(cmdArgs->args[1]);
	
	if(smash->job_array->jobs[job_id] == NULL) {
		printf("smash error: bg: job id %d does not exist\n", job_id);
		return 1;
	}
	else if(smash->job_array->jobs[job_id]->is_stopped) {
		printf("%s: %d\n", smash->job_array->jobs[job_id]->cmd, smash->job_array->jobs[job_id]->pid);
		kill(smash->job_array->jobs[job_id]->pid, SIGCONT);
		smash->job_array->jobs[job_id]->is_stopped = false;
		return 0;
	}
	else if(smash->job_array->jobs[job_id]->is_stopped == false) {
		printf("smash error: bg: job id %d is already in background\n", job_id);
		return 1;
	}
	return 0;

}


int fg(CmdArgs_t* cmdArgs) {
	if(cmdArgs == NULL) {
		printf("smash error: fg: invalid arguments\n");
		return 1;
	}
	int argc = cmdArgs->nargs;
	
	if(argc == 1) {
		for(int i = JOBS_NUM_MAX-1; i >= 0; i--) {
			if(smash->job_array->jobs[i] != NULL) {
				if(smash->job_array->jobs[i]->is_stopped) {
					strcpy(cmd_last_command , smash->job_array->jobs[i]->cmd);
					kill(smash->job_array->jobs[i]->pid, SIGCONT);
					smash->job_array->jobs[i]->is_stopped = false;
				}
				printf("[%d] %s\n", smash->job_array->jobs[i]->job_id, smash->job_array->jobs[i]->cmd);
				strcpy(cmd_last_command , smash->job_array->jobs[i]->cmd);

				smash->fg_pid = smash->job_array->jobs[i]->pid;
				remove_job(smash->job_array, smash->job_array->jobs[i]);
				return 0;
			}
		
		if(i==0) {
			printf("smash error: fg: jobs list is empty\n"); 
			return 1;
		}
	}
}
	
	if(argc > 2) {
		printf("smash error: fg: invalid arguments\n");
		return 1;
	}

	int status_number = is_valid_number(cmdArgs->args[1]);
	if(status_number == WITH_LETTER) {
		printf("smash error: fg: invalid arguments\n");
		return 1;
	}
	else if(status_number == INVALID_DIGIT) {
		printf("smash error: fg: job id %s does not exist\n", cmdArgs->args[1]);
		return 1;
	}

	int job_id = atoi(cmdArgs->args[1]);
	
	if(smash->job_array->jobs[job_id] == NULL) {
		printf("smash error: fg: job-id %d does not exist\n", job_id);
		return 1;
	}
	if(smash->job_array->jobs[job_id]->is_stopped) {
		strcpy(cmd_last_command , smash->job_array->jobs[job_id]->cmd);
		kill(smash->job_array->jobs[job_id]->pid, SIGCONT);
		smash->job_array->jobs[job_id]->is_stopped = false;
	}

	printf("[%d] %s\n", smash->job_array->jobs[job_id]->job_id, smash->job_array->jobs[job_id]->cmd);
	strcpy(cmd_last_command , smash->job_array->jobs[job_id]->cmd);
	smash->fg_pid = smash->job_array->jobs[job_id]->pid;
	remove_job(smash->job_array, smash->job_array->jobs[job_id]);
	return 0;
}

int kill_command(CmdArgs_t* cmdArgs) {
	if(cmdArgs == NULL) {
		printf("smash error: kill: invalid arguments\n");
		return 1;
	}
	int argc = cmdArgs->nargs;
	if(argc !=3 ) {
		printf("smash error: kill: invalid arguments\n");
		return 1;
	}


	int status_number_1 = is_valid_number(cmdArgs->args[1]);
	int status_number_2 = is_valid_number(cmdArgs->args[2]);
	if(status_number_1 == WITH_LETTER || status_number_2 == WITH_LETTER) {
		printf("smash error: kill: invalid arguments\n");
		return 1;
	}

	int signal_num = atoi(cmdArgs->args[1]);
    int job_id = atoi(cmdArgs->args[2]);
	if(job_id == INVALID_DIGIT || smash->job_array->jobs[job_id] == NULL) {
		printf("smash error: kill: job id %s does not exist\n", cmdArgs->args[2]);
		return 1;
	}


	// if(kill(smash->job_array->jobs[job_id]->pid, signal_num) == -1) {
	// 	printf("smash error: kill failed\n");
	// 	return 1;
	// }
	//maybe check if signal number is valid
	if(kill(smash->job_array->jobs[job_id]->pid, signal_num) == -1) {
		printf("smash error: kill: invalid arguments\n");
		return 1;
	}
	printf("signal number %d was sent to pid %d\n", signal_num, smash->job_array->jobs[job_id]->pid);
	return 0;
}

	int quit(CmdArgs_t* cmdArgs) {
		if(cmdArgs == NULL) {
			printf("smash error: quit: invalid arguments\n");
			return 1;
		}
		int argc = cmdArgs->nargs;
		if(argc == 1) {
			 exit(0);
		}
		else if(argc >2) {
			printf("smash error: quit: expected 0 or 1 arguments\n");
			return 1;
		}
		else {
			if (strcmp(cmdArgs->args[1], "kill") != 0) {
				printf("smash error: quit: unexpected arguments\n");
				return 1;
			}
			else {
				for(int i = 0; i < JOBS_NUM_MAX; i++) {
					check_finished_jobs();
					if(smash->job_array->jobs[i] != NULL) {
						bool is_killed = false;
						if(kill(smash->job_array->jobs[i]->pid, SIGTERM) == -1) {
							printf("smash error: kill failed\n");
							return 1;
						}
						printf("[%d] %s – sending SIGTERM… ", smash->job_array->jobs[i]->job_id, smash->job_array->jobs[i]->cmd);
						fflush(stdout);
						for(int j=0 ; j<5;j++){
							if(!(waitpid(smash->job_array->jobs[i]->pid, NULL,WNOHANG) > 0)) {
								
								sleep(1);
							}
							else{
								is_killed = true;
								break;
							}
						}
						if(!is_killed){
							
							if(kill(smash->job_array->jobs[i]->pid, SIGKILL) == -1) {
								printf("smash error: kill failed\n");
								return 1;
							}
							printf("sending SIGKILL… ");
						}
						printf("done\n");
					}
				}
				exit(0);
			}
		}
		return 0;
	}

int diff(CmdArgs_t* cmdArgs) {
	if(cmdArgs == NULL) {
		printf("smash error: diff: 	expected 2 arguments\n");
		return 1;
	}
	int argc = cmdArgs->nargs;
	if(argc != 3 ) {
		printf("smash error: diff: expected 2 arguments\n");
		return 1;
	}
	
	int type1 = check_path_type(cmdArgs->args[1]);
	int type2 = check_path_type(cmdArgs->args[2]);

	if(type1 == NOT_FOUND || type2 == NOT_FOUND) {
		printf("smash error: diff: expected valid paths for files\n");
		return 1;
	}
	if(type1 == DIR_TYPE || type2 == DIR_TYPE) {
		printf("smash error: diff: paths are not files\n");
		return 1;
	}
	
	FILE* file1 = fopen(cmdArgs->args[1], "r");
	FILE* file2 = fopen(cmdArgs->args[2], "r");
	if(file1 == NULL || file2 == NULL) {
		printf("smash error: diff: paths are not files\n");
		return 1;
	}
	
	char ch1, ch2;
	while(1) {
		ch1 = fgetc(file1);
		ch2 = fgetc(file2);
		
		// If both files reached EOF, they are identical
		if(ch1 == EOF && ch2 == EOF) {
			printf("0\n");
			fclose(file1);
			fclose(file2);
			return 0;
		}
		
		// If only one file reached EOF or characters differ
		if(ch1 != ch2) {
			printf("1\n");
			fclose(file1); 
			fclose(file2);
			return 0;
		}
	}

	
	
}

//

void ctrl_z_handler(int sig) {
    printf("smash: caught CTRL+Z\n");
	if (smash->fg_pid == 0) {
		longjmp(env_buf, 1);
	}
	
    if (smash->fg_pid != 0) {
        if (kill(smash->fg_pid, SIGSTOP) == -1) {
            perror("smash error: kill failed");
            return;
        }
		job_t* job = NULL;
		if(strcmp(cmdArgs->args[0], "fg")!=0) {
			job = create_job(smash->fg_pid,
								cmdArgs->cmd,
								 true, 0, false);
			
		 } 
		 else{
			
			job = create_job(smash->fg_pid,
								cmd_last_command,
								 true, 0, true);
		 }
	
        insert_job(smash->job_array, job);

        printf("smash: process %d was stopped\n", smash->fg_pid);
        smash->fg_pid = 0;
    }
}
	
	void ctrl_c_handler(int sig) {
		printf("smash: caught CTRL+C\n");
	
		if (smash->fg_pid == 0) {
			// Smash itself is running a built-in command
			longjmp(env_buf, 1);  // Jump back to the main prompt loop
		} else {
			// Another process is running in foreground
			if (kill(smash->fg_pid, SIGKILL) == -1) {
				perror("smash error: kill failed");
			} else {
				printf("smash: process %d was killed\n", smash->fg_pid);
			}
			smash->fg_pid = 0;
		}
	}
	


void init_signal_handlers() {
    struct sigaction sa_z;
    sa_z.sa_handler = ctrl_z_handler;
    sigemptyset(&sa_z.sa_mask);
    sa_z.sa_flags = 0;  // Changed to 0 for simplicity
    if (sigaction(SIGTSTP, &sa_z, NULL) == -1) {
        perror("sigaction");
    }

	struct sigaction sa_c;
    sa_c.sa_handler = ctrl_c_handler;
    sigemptyset(&sa_c.sa_mask);
    sa_c.sa_flags = 0;

    if (sigaction(SIGINT, &sa_c, NULL) == -1) {
        perror("smash error: sigaction SIGINT failed");
    }
}



bool is_builtin(const char* cmd){
	return (strcmp(cmd, "pwd")==0)
	 ||strcmp(cmd, "showpid")==0 
	 ||strcmp(cmd, "cd")==0 
	 ||strcmp(cmd, "jobs")==0 
	 ||strcmp(cmd, "bg") ==0
	 ||strcmp(cmd, "fg")==0
	 ||strcmp(cmd, "kill")==0
	 ||strcmp(cmd, "quit")==0
	 ||strcmp(cmd, "diff")==0;
}

int commandHandler(CmdArgs_t* cmdArgs){
	if(strcmp(cmdArgs->args[0], "pwd" ) == 0)
		return pwd(cmdArgs);
	else if(strcmp(cmdArgs->args[0], "showpid" ) == 0)
		return showpid(cmdArgs);
	else if(strcmp(cmdArgs->args[0], "cd" ) == 0)
		return cd(cmdArgs);
	else if(strcmp(cmdArgs->args[0], "jobs" ) == 0)
		return jobs(cmdArgs);
	else if(strcmp(cmdArgs->args[0], "bg")==0)
		return bg(cmdArgs);
	else if(strcmp(cmdArgs->args[0], "fg")==0)
		return fg(cmdArgs);
	else if (strcmp(cmdArgs->args[0], "kill")==0)	
		return kill_command(cmdArgs);
	else if (strcmp(cmdArgs->args[0], "quit")==0)
		return quit(cmdArgs);
	else if (strcmp(cmdArgs->args[0], "diff")==0)
		return diff(cmdArgs);
		
	else return 1;	
}
			
void check_finished_jobs() {
    int status;
    pid_t pid;// Loop over all jobs
    for (int i = 0; i< JOBS_NUM_MAX; ++i) {
        if (smash->job_array->jobs[i] != NULL) {
            pid = waitpid(smash->job_array->jobs[i]->pid, &status, WNOHANG);
            if (pid > 0) {
                remove_job(smash->job_array, smash->job_array->jobs[i]);
                --i; // to handle shifting if you compact the array after removal
            }
        }
    }
	//print_all_jobs(smash->job_array);
}

/*=============================================================================
* global variables & data structures
=============================================================================*/
char _line[CMD_LENGTH_MAX];

/*=============================================================================
* main function
=============================================================================*/
int main(int argc, char* argv[]) {
    char _cmd[CMD_LENGTH_MAX];
    init_smash();
	cmd_last_command[0] = '\0';
    // Ignore SIGTSTP for the smash process itself
    while (1) {
		sigsetjmp(env_buf, 1);
        printf("smash > ");
        fflush(stdout);  // Ensure prompt shows up immediately
        tcflush(STDIN_FILENO, TCIFLUSH);
        if (!fgets(_line, CMD_LENGTH_MAX, stdin)) {
            break;  // EOF or error
        }

        strcpy(_cmd, _line);
		
        cmdArgs = parseCmd(_cmd);
        if (cmdArgs == NULL || cmdArgs->args[0] == NULL) {
            freeCmdArgs(cmdArgs);
            continue;
        }

        if (is_builtin(cmdArgs->args[0])) {
			
            if (cmdArgs->bg) {
                pid_t pid = fork();
                if (pid == 0) {
                    setpgid(0, 0);
                    int status = commandHandler(cmdArgs);
                    exit(status);
                } else {
                    check_finished_jobs();  // Remove ended jobs before inserting
                    job_t* job = create_job(pid, cmdArgs->cmd, false, 0, true);
                    insert_job(smash->job_array, job);
                }
            } else {
                if (!commandHandler(cmdArgs)) {
				
                    if (smash->fg_pid != 0) {
                        waitpid(smash->fg_pid, NULL, 0);
                        smash->fg_pid = 0;
                    }
                }
            }
        } else {
			// Check if command exists as a file (relative or absolute path)
			if (access(cmdArgs->args[0], F_OK) == -1) {
				fprintf(stderr, "smash error: external: cannot find program\n");
				longjmp(env_buf, 1);  // Jump back to the shell prompt
			}
		
			pid_t pid = fork();
			if (pid == 0) {
				setpgid(0, 0);
				execvp(cmdArgs->args[0], cmdArgs->args);
		
				// execvp failed
				if (errno == ENOENT) {
					fprintf(stderr, "smash error: external: cannot find program\n");
				} else {
					fprintf(stderr, "smash error: external: invalid command\n");
				}
				exit(1);
			} else {
				if (cmdArgs->bg) {
					check_finished_jobs();  // Remove ended jobs before inserting
					job_t* job = create_job(pid, cmdArgs->cmd, false, 0, true);
					insert_job(smash->job_array, job);
				} else {
					smash->fg_pid = pid;
					waitpid(pid, NULL, 0);
					smash->fg_pid = 0;
				}
			}
		}
		

        _line[0] = '\0';
        _cmd[0] = '\0';
        freeCmdArgs(cmdArgs);
    }

    delete_smash();
    return 0;
}
