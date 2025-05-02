#include "commands.h"

//=============================================================================
/**
 * @brief global variables to track fg process that are defined in smash.c
 */

 char prev_path[PATH_MAX] ="";

 //==============================================================================

 // helper functions

 // change char to int for job id
 int str_to_int(char *str)
 {
	 int num = 0;
	 for (int i = 0; str[i] != '\0'; i++)
	 {
		 if (str[i] < '0' || str[i] > '9')
		 {
			 return -1;
		 }
		 num = num * 10 + (str[i] - '0');
	 }
	 if (num < 0 || num >= MAX_JOBS)
	 {
		 return -1;
	 }
	 return num;
 }

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
	if (getcwd(cwd, sizeof(cwd)) != NULL) { //getcwd prints the current working directory
		return SMASH_SUCCESS;
	} else {
		perror("getcwd() error"); //won't happen - just for clarity
	}
	return SMASH_ERROR;
}

//if comes with & need to see that we send the correct procces in the main to this function
int cd(char* path)
{
	if (path == NULL) {
		fprintf(stderr, "error: cd: target directory does not exist\n");
		//perhaps need this print instead:     fprintf(stderr, "smash error: cd: expected 1 arguments\n");
		return SMASH_FAIL;
	}
	char curr_cwd[PATH_MAX]; 
	if (getcwd(curr_cwd, sizeof(curr_cwd)) == NULL){ 
		perror("getcwd() error");
		return SMASH_ERROR;
	}
	// Handle "cd -"
	if (strcmp(path, "-") == 0) { 
		if (strlen(prev_path) == 0) {
			fprintf(stderr, "smash error: cd: old pwd not set\n");
			return SMASH_FAIL;
		}
		char temp[PATH_MAX];
		getcwd(temp, sizeof(temp)); //get the current working directory
		if (chdir(prev_path) != 0) {
			perror("smash error: cd");
			return SMASH_FAIL;
		}
		printf("%s\n", prev_path);
		strcpy(prev_path, temp); //update prev_path to the current working directory
		return SMASH_SUCCESS;
	}
	//handle "cd.."
	else if (strcmp(path, "..") == 0) {	
		if (strcmp(fg_cmd_line, "/") == 0) { 
			getcwd(prev_path, sizeof(prev_path)); //perhaps not good
			return SMASH_SUCCESS; //need to see whats going of with prev path - might have a bug here
		}
		getcwd(prev_path, sizeof(prev_path)); 
		if (chdir("..") != 0) {
			perror("smash error: cd");
			return SMASH_FAIL;
		}
		return SMASH_SUCCESS;
	}
	// normal case
	struct stat path_stat;
    if (stat(path, &path_stat) == 0) { 		// Check if the path exists
        if (S_ISDIR(path_stat.st_mode)) { 	// Path exists, check if it's a directory
            getcwd(prev_path, sizeof(prev_path));
			if (chdir(path) != 0) {
                perror("smash error: cd");
            }
        }
		else {
            fprintf(stderr, "smash error: cd: %s: Not a directory\n", path);	// Path exists, but it's not a directory (it's a file)
			return SMASH_FAIL;
        }
    } else {
        fprintf(stderr, "smash error: cd: target directory does not exist\n");	// Path doesn't exist
		return SMASH_FAIL;
    }
	return SMASH_SUCCESS;
}

//should be fine - not yet tested though
int jobs(){
	for (int i = 0; i < MAX_JOBS; i++) {
		if (get_pid(jobs_arr[i]) != 0) {
			print_job(i); //every reference to the array should be according to job id
		}
	}
	return SMASH_SUCCESS;
}

//should be fine - not yet tested though
int smash_kill(int signum, char* job_id){
	int job_index = str_to_int(job_id); 
	if (job_index == -1) {
		fprintf(stderr, "smash error: kill: invalid arguments\n");
		return SMASH_FAIL;
	}
	if (get_pid(jobs_arr[job_index]) == 0) {
		fprintf(stderr, "smash error: kill: job id %d does not exist\n", job_index);
		return SMASH_FAIL;	//perhaps need to be SMASH_ERROR

	}
	
	pid_t job_pid = get_pid(jobs_arr[job_index]);
	if (kill(job_pid, signum) == -1) {
        perror("smash error: kill");
		return SMASH_ERROR;
    } else {
        printf("signal %d sent to pid %d\n", signum, job_pid); 
    }
	return SMASH_SUCCESS;
}

//should be fine - not yet tested though
int empty_fg(){
	int max_id = -1;
	for (int i = 0; i < MAX_JOBS; i++) { 
		if (get_pid(jobs_arr[i]) != 0) {
			max_id = i;
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
	if (get_pid(jobs_arr[job_index]) == 0) {
		fprintf(stderr, "smash error: fg: job id %d does not exist\n", job_index);
		return SMASH_FAIL; //perhaps need to be SMASH_ERROR
	}
	pid_t job_pid = get_pid(jobs_arr[job_index]);
	if (kill(job_pid, SIGCONT) == -1) {
		perror("smash error: fg: kill failed");
		return SMASH_ERROR;
	}
	//update global
	fg_pid = job_pid;
    strcpy(fg_cmd_line, get_cmd_line(jobs_arr[job_index]));
	//remove from jobs_arr
	print_job(job_index);
	remove_job(job_index);
	//wait for the job to finish
	int status;
	if (waitpid(job_pid, &status, WUNTRACED) == -1) {
		perror("smash error: fg: waitpid failed");
		return SMASH_ERROR;
	}
	if (WIFEXITED(status) || WIFSIGNALED(status)) {
		fg_pid = -1;
	} else if (WIFSTOPPED(status)){
		add_job(job_pid, fg_cmd_line, JOB_STOPPED);
		fg_pid = -1;
	}
	return SMASH_SUCCESS;
}


//should be fine - not yet tested though
int empty_bg(){
	int max_id = -1;
	for (int i = 0; i < MAX_JOBS; i++) {
		if (get_pid(jobs_arr[i]) != 0) {
			if (jobs_arr[i].status == JOB_STOPPED) {
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
	if (get_pid(jobs_arr[job_index]) == 0) {
		fprintf(stderr, "smash error: bg: job id %d does not exist\n", job_index);
		return SMASH_FAIL; //perhaps need to be SMASH_ERROR
	}
	if (jobs_arr[job_index].status == JOB_RUNNING_BG) { //mabey alon called it another name + perhaps need helper func and not arrow
		fprintf(stderr, "smash error: bg: job id %d is already in background\n", job_index);
		return SMASH_FAIL; //perhaps need to be SMASH_ERROR
	}
	pid_t job_pid = get_pid(jobs_arr[job_index]);
	if (kill(job_pid, SIGCONT) == -1) {
		perror("smash error: fg: kill failed");
		return SMASH_ERROR;
	}	
	update_job_status(job_index, JOB_RUNNING_BG);
	print_job(job_index);
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
		if (get_pid(jobs_arr[i]) != 0) {
			int curr_id = i; //TODO: unused
			//pid_t curr_pid = get_pid(jobs_arr[i]); //perhaps helper func //TODO: unused
			char* cmd = get_cmd_line(jobs_arr[i]);
			printf("[%d] %s - sending SIGTERM... ", curr_id, cmd);
			if (kill(get_pid(jobs_arr[i]), SIGTERM) == -1) { //TODO:  I have changed job_pid -> jobs_arr[i].pid. job_id was undeclared
                perror("smash error: quit kill - SIGTERM failed");//wont happend - if we see this print just delet the print line
                continue;
            }
					
		}
		time_t start_time = time(NULL);
		// Wait up to 5 seconds
		while (time(NULL) - start_time < 5) {
			if (waitpid(get_pid(jobs_arr[i]), NULL, WNOHANG) > 0) { //TODO:  I have changed job_pid -> jobs_arr[i].pid. job_id was undeclared
				printf("done\n");
				break;
			}
			sleep(1); // sleep 1 second between checks
		}

		// If still alive after 5 seconds
		if (waitpid(get_pid(jobs_arr[i]), NULL, WNOHANG) == 0) {
			
			printf("sending SIGKILL... done\n");
			if (kill(get_pid(jobs_arr[i]), SIGKILL) == -1) { //TODO:  I have changed job_pid -> jobs_arr[i].pid. job_id was undeclared
				perror("smash error: quit kill - SIGKILL failed"); //wont happend - if we see this print just delet the print line
				return SMASH_ERROR;
			}
		}

	}
	clean_all_jobs(); //TODO: is this the right location?
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


