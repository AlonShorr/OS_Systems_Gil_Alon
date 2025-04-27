#include "commands.h"

#define SMASH_SUCCESS 0
#define SMASH_FAIL 1
#define SMASH_ERROR 2
#define MAX_JOBS 100
#define PATH_MAX 4096
#define JOB_RUNNUNG_FG 0
#define JOB_RUNNUNG_BG 1
#define JOB_STOPPED 2

char prev_path[PATH_MAX] ="";

//=============================================================================
/*
//example function for printing errors from internal commands
void perrorSmash(const char* cmd, const char* msg)
{
    fprintf(stderr, "smash error:%s%s%s\n",
        cmd ? cmd : "",
        cmd ? ": " : "",
        msg);
}

//example function for parsing commands
int parseCmdExample(char* line)
{
	char* delimiters = " \t\n"; //parsing should be done by spaces, tabs or newlines
	char* cmd = strtok(line, delimiters); //read strtok documentation - parses string by delimiters
	if(!cmd)
		return INVALID_COMMAND; //this means no tokens were found, most like since command is invalid
	
	char* args[MAX_ARGS];
	int nargs = 0;
	args[0] = cmd; //first token before spaces/tabs/newlines should be command name
	for(int i = 1; i < MAX_ARGS; i++)
	{
		args[i] = strtok(NULL, delimiters); //first arg NULL -> keep tokenizing from previous call
		if(!args[i])
			break;
		nargs++;
	}
	/*
	At this point cmd contains the command string and the args array contains
	the arguments. You can return them via struct/class, for example in C:
		typedef struct {
			char* cmd;
			char* args[MAX_ARGS];
		} Command;
	Or maybe something more like this:
		typedef struct {
			bool bg;
			char** args;
			int nargs;
		} CmdArgs;
	*/
//}
//=============================================================================

// requaired functions

//should be fine - not yet tested though
int showpid()
{
	pid_t pid = getpid();
	printf("smash pid is %d\n", pid);
	return SMASH_SUCCESS;
}

//should be fine - not yet tested though
int pwd()
{
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("%s\n", cwd);
		return SMASH_SUCCESS;
	} else {
		perror("getcwd() error"); //won't happen - just for clearty
	}
	return SMASH_ERROR;
}

// need to check this, might have bugs in the way we print/following the "-" command in certine cases
//if comes with & need to see that we send the correct procces in the main to this function
int cd(char* path)
{
	if (path == NULL) {
		fprintf(stderr, "error: cd: target directory does not exist\n");
		//perhaps need this print instead:     fprintf(stderr, "smash error: cd: expected 1 arguments\n");
		return SMASH_FAIL;
	}
	char curr_cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) == NULL){
		perror("getcwd() error");
		return SMASH_ERROR;
	}

// Handle "cd -"
	if (strcmp(arg, "-") == 0) {
		if (strlen(prev_path) == 0) {
			fprintf(stderr, "smash error: cd: old pwd not set");
			return SMASH_FAIL;
		}

		if (chdir(prev_path) != 0) {
			perror("smash error: cd");
			return SMASH_FAIL;
		}

		printf("%s\n", prev_path);
	}
	
	//handle "cd.."
	else if (strcmp(path, "..") == 0) {
		if (strcmp(curr_path, "/") == 0) {
			return SMASH_SUCCESS; //need to see whats going of with prev path - might have a bug here
		}
		if (chdir("..") != 0) {
			perror("smash error: cd");
			return SMASH_FAIL;
		}
	}

	// normal case

	//probebly bad: we needed specific prints thats why i made longer version and not just perror
	/* 
	else if (chdir(path) != 0) {
		perror("smash error: cd");
		//idk if perror can distinguish between "target directory does not exist" and "path + not a directory". need to check in debug
		return SMASH_FAIL;
	}
	*/

	// probebly good:
	struct stat path_stat;
    if (stat(path, &path_stat) == 0) { 		// Check if the path exists
        if (S_ISDIR(path_stat.st_mode)) { 	// Path exists, check if it's a directory
            if (chdir(path) != 0) {
                perror("smash error: cd");
            }
        } else {
            fprintf(stderr, "smash error: cd: %s: Not a directory\n", path);	// Path exists, but it's not a directory (it's a file)
			return SMASH_FAIL;
        }
    } else {
        fprintf(stderr, "smash error: cd: target directory does not exist\n");	// Path doesn't exist
		return SMASH_FAIL;
    }

	// update prev_path
	strcpy(prev_path, curr_path); //perhaps need to be in defferent place - check in debug
	return SMASH_SUCCESS;
}

//should be fine - not yet tested though
int jobs(){
	for (int i = 0; i < MAX_JOBS; i++) {
		if (jobs_arr[i]!= NULL) {
			print_job(jobs_arr[i]);
		}
	}
	return SMASH_SUCCESS;
}

//should be fine - not yet tested though
int kill(int signum, char* job_id){
	int job_index = str_to_int(job_id);
	if (job_index == -1) {
		fprintf(stderr, "smash error: kill: invalid arguments\n");
		return SMASH_FAIL;
	}
	if (jobs_arr[job_index] == NULL) {
		fprintf(stderr, "smash error: kill: job id %d does not exist\n", job_index);
		return SMASH_FAIL;	//perhaps need to be SMASH_ERROR

	}
	
	pid_t job_pid = jobs_arr[job_index].pid;
	if (kill(job_pid, signum) == -1) {
        perror("smash error: kill");
		return SMASH_ERROR;
    } else {
        printf("signal %d sent to pid %d\n", signal, job_pid);
    }
	return SMASH_SUCCESS;
}

//should be fine - not yet tested though
int empty_fg(){
	int max_id = -1;
	for (int i = 0; i < MAX_JOBS; i++) {
		if (jobs_arr[i] != NULL) {
			Max_id = i;
			}
		}
	if (max_id == -1) {
		fprintf(stderr, "smash error: fg: jobs list is empty\n");
		return SMASH_FAIL; //perhaps need to be SMASH_ERROR
	}
	char job_id_char[3] ="";
	sprintf(job_id_char, "%02d", max_id);
	return fg(job_id_char);
}
int fg(char* job_id){
	int job_index = str_to_int(job_id);
	if (job_index == -1) {
		fprintf(stderr, "smash error: fg: invalid arguments\n");
		return SMASH_FAIL;
	}
	if (jobs_arr[job_index] == NULL) {
		fprintf(stderr, "smash error: fg: job id %d does not exist\n", job_index);
		return SMASH_FAIL; //perhaps need to be SMASH_ERROR
	}
	send_fg_signal(job_index); //alon write - it send signal and update the arr
	print_job(jobs_arr[job_index]);
	return SMASH_SUCCESS;
}

//should be fine - not yet tested though
int empty_bg(){
	int max_id = -1;
	for (int i = 0; i < MAX_JOBS; i++) {
		if (jobs_arr[i] != NULL) {
			if (jobs_arr[i].status == JOB_RUNNUNG_BG) { //mabey alon called it another name + perhaps need helper func and not arrow
				max_id = i;
			}
		}
	}
	if (max_id == -1) {
		fprintf(stderr, "smash error: bg: there are no stopped jobs to resume\n");
		return SMASH_ERROR;
	}
	char job_id_char[3] ="";
	sprintf(job_id_char, "%02d", max_id);
	return bg(job_id_char);
}
int bg(char* job_id){
	int job_index = str_to_int(job_id);
	if (job_index == -1) {
		fprintf(stderr, "smash error: bg: invalid arguments\n");
		return SMASH_FAIL;
	}
	if (jobs_arr[job_index] == NULL) {
		fprintf(stderr, "smash error: bg: job id %d does not exist\n", job_index);
		return SMASH_FAIL; //perhaps need to be SMASH_ERROR
	}
	if (jobs_arr[job_index].status == JOB_RUNNUNG_BG) { //mabey alon called it another name + perhaps need helper func and not arrow
		fprintf(stderr, "smash error: bg: job id %d is already in background\n", job_index);
		return SMASH_FAIL; //perhaps need to be SMASH_ERROR
	}
	send_bg_signal(job_index); //alon write - it send signal and update the arr
	print_job(jobs_arr[job_index]);
	return SMASH_SUCCESS;
}

//should be fine - not yet tested though
int quit(){
	exit(0);
	return SMASH_SUCCESS;
}
int quit_kill(){
	//kill all jobs
	for (int i = 0; i < MAX_JOBS; i++) {
		if (jobs_arr[i] != NULL) {
			int curr_id = i;
			pid_t curr_pid = jobs_arr[i].pid; //perhaps helper func
			char* cmd = jobs_arr[i].cmd_line;
			printf("[%d] %s - sending SIGTERM... ", curr_id, cmd);
			
			if (kill(job_pid, SIGTERM) == -1) {
                perror("smash error: quit kill - SIGTERM failed");//wont happend - if we see this print just delet the print line
                continue;
            }
					
		}
		time_t start_time = time(NULL);
		// Wait up to 5 seconds
		while (time(NULL) - start_time < 5) {
			if (waitpid(job_pid, NULL, WNOHANG) > 0) {
				printf("done\n");
				break;
			}
			sleep(1); // sleep 1 second between checks
		}

		// If still alive after 5 seconds
		if (waitpid(job_pid, NULL, WNOHANG) == 0) {
			
			printf("sending SIGKILL... done\n");
			if (kill(job_pid, SIGKILL) == -1) {
				perror("smash error: quit kill - SIGKILL failed"); //wont happend - if we see this print just delet the print line
				return SMASH_ERROR;
			}
		}

	}
	exit(0);
	return SMASH_SUCCESS;
}

//should be fine - not yet tested though
int diff(char* file1, char* file2){
	// Check if the paths are valid
	if (file1 == NULL || file2 == NULL) {
		fprintf(stderr, "smash error: diff: expected 2 arguments\n");
		return SMASH_FAIL;
	}	
	struct stat path_stat1;
	struct stat path_stat2;
	// Check if the paths exist
	if (stat(file1, &path_stat1) != 0 || stat(file2, &path_stat2) != 0) { 
		fprintf(stderr, "smash error: diff: expected valid paths for files\n");
		return SMASH_FAIL;
	}
	// Check if the paths are directories
	if (S_ISDIR(path_stat1.st_mode) || S_ISDIR(path_stat2.st_mode)) { 
		fprintf(stderr, "smash error: diff: paths are not files\n");
		return SMASH_FAIL; //perhaps need to be SMASH_ERROR
	}

	//compare bit by bit
	FILE *fp1 = fopen(file1, "rb");
    FILE *fp2 = fopen(file2, "rb");

    if (!fp1 || !fp2) {
		fprintf(stderr, "smash error: diff: expected valid paths for files\n");
        if (fp1) fclose(fp1);
        if (fp2) fclose(fp2);
        return SMASH_ERROR;  // error opening one of the files
    }
    int ch1, ch2;
    do {
        ch1 = fgetc(fp1);
        ch2 = fgetc(fp2);
        if (ch1 != ch2) {
            fclose(fp1);
            fclose(fp2);
            return SMASH_ERROR;  // Files are different
        }
    } while (ch1 != EOF && ch2 != EOF);

    fclose(fp1);
    fclose(fp2);

    if (ch1 == EOF && ch2 == EOF) {
        return SMASH_SUCCESS;  // Files are identical
    } else {
        return SMASH_ERROR;  // One file ended earlier -> files are different
    }
}

//==============================================================================
// helper functions

//change char to int for job id
int str_to_int(char* str)
{
	int num = 0;
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] < '0' || str[i] > '9') {
			return -1;
		}
		num = num * 10 + (str[i] - '0');
	}
	if (num < 0 || num >= MAX_JOBS) {
		return -1;
	}
	return num;
}
