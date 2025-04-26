//jobs.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/

#include "jobs.h"

/**
 * @param jobs_arr a program global array that holds the jobs queue. The array
 * indexes 0 for an empty spot. For an index containing a job job_id == it's index  
 * in the array.
 */
job jobs_arr[MAX_JOBS];

/**
 * @param jobs_nums: global number of jobs currently processed.
 */
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
    
    job* new = (job*)arr[index];
    
    new.job_id = index;
    new.pid = pid;
    new.cmd_line = (char*)malloc(sizeof(char)*(strlen(cmd_line) + 1));
    strcpy(new.cmd_line, cmd_line);
    new.start_time = /*insert current clock time*/; //TODO
    new.seconds_elapsed = /*0? figure this out*/; 
    new.status = status;

    jobs_num++;

    return job_id; //on Success
}

 void remove_job(int job_id) {
    if (job_id < 0 || job_id >= MAX_JOBS) 
        return;
    else if (jobs_arr[job_id].pid == 0) 
        return;  

    job *job = jobs_arr[job_id];
    free(job->cmd_line);
    memset(job, 0, sizeof(job));

    jobs_num--;
}

