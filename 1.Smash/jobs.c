//jobs.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/

#include "jobs.h"


job jobs_arr[MAX_JOBS];
int jobs_num = 0;

/*=============================================================================
* Function Declarations
=============================================================================*/

/**
 * @brief A new job will always get the new minimal availible job_id == the lowest
 * index in the array containing zero. This function searches for the next job_id
 * to give an incoming job.
 * @return returns the next free index for a job (next zeroed index).
 */
int next_free_index() {
    for(int i=0; i < MAX_JOBS; i++) {
        if (jobs_arr[i].pid == 0)  
            return i;
    }

    return ERROR;
}


void init_jobs() {
    memset(jobs_arr, 0, sizeof jobs_arr);
    jobs_num = 0;
}


int add_job(int pid, const char *cmd_line, int status) {
    int index = next_free_index();
    if (index == ERROR)
        return ERROR;
    
    job* new_job = &jobs_arr[index];
    
    new_job->cmd_line = (char*)malloc(sizeof(char)*(strlen(cmd_line) + 1));
    if(!new_job->cmd_line) return ERROR;
    strcpy(new_job.cmd_line, cmd_line);
    new_job->job_id = index;
    new_job->pid = pid;
    new_job->start_time = time(NULL); //TODO: check if it works good
    new_job->seconds_elapsed = 0;
    new_job->status = status;

    jobs_num++;

    return index; //on Success
}

 void remove_job(int job_id) {
    if (job_id < 0 || job_id >= MAX_JOBS) 
        return;
    else if (jobs_arr[job_id].pid == 0) 
        return;  

    job *job = &jobs_arr[job_id];
    if(job->cmd_line)
        free(job->cmd_line);
    memset(job, 0, sizeof(*job));

    jobs_num--;
}

void print_job()

//TODO TIME MANAGMENT 
//PRINT FUNCTION FOR ONE JOB ELEMNT!!!!!!!!!!!!!!!!!!!!!!! print_job
