//jobs.c

/*=============================================================================
* includes, defines, usings
=============================================================================*/

#include "jobs.h"

#ifndef WCONTINUED
#define WCONTINUED 0
#endif

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

int get_id (int pid) {
    for(int i=0; i < MAX_JOBS; i++) {
        if (jobs_arr[i].pid == pid)  
            return i;
    }
    return ERROR;
}


int get_pid (int job_id) {
    if (job_id < 0 || job_id >= MAX_JOBS) 
        return ERROR;
    else if (jobs_arr[job_id].pid == 0) 
        return ERROR;  

    return jobs_arr[job_id].pid;
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
    strcpy(new_job->cmd_line, cmd_line);
    new_job->job_id = index;
    new_job->pid = pid;
    new_job->start_time = time(NULL);
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

void print_job(int job_id) {
    if ((job_id < 0 || job_id >= MAX_JOBS) 
         || jobs_arr[job_id].pid == 0
         || jobs_arr[job_id].status == JOB_RUNNING_FG)
        return;

    job *job = &jobs_arr[job_id];
    long elapsed = (long)(time(NULL) - job->start_time);

    if(job->status == JOB_STOPPED)
        printf("[%d] %s: %d %ld (stopped)\n",job->job_id, 
                                             job->cmd_line,
                                             job->pid,
                                             elapsed); 
    else if(job->status == JOB_RUNNING_BG)
        printf("[%d] %s: %d %ld\n",job->job_id, 
                                   job->cmd_line,
                                   job->pid,
                                   elapsed); 
}

void update_job_status(int job_id, int status) {
    if (job_id < 0 || job_id >= MAX_JOBS) 
        return;
    else if (jobs_arr[job_id].pid == 0) 
        return;  

    jobs_arr[job_id].status = status;
    if (status == JOB_RUNNING_BG)               //TODO: do we need to reset the time?
        jobs_arr[job_id].start_time = time(NULL);
           
    return;
}

void update_jobs() {
    int status, pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        int job_id = get_id(pid);
        if (WIFEXITED(status) || WIFSIGNALED(status)) { //child terminated by signal/exit
            remove_job(job_id);
        }
        else if (WIFSTOPPED(status)) {                 // child had sttoped
            update_job_status(job_id, JOB_STOPPED);
        }
        else if (WIFCONTINUED(status)) {               // child resumed
            update_job_status(job_id, JOB_RUNNING_BG);
        }
    }
}

void clean_all_jobs(){
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_arr[i].pid != 0) {
            free(jobs_arr[i].cmd_line);
            jobs_arr[i].cmd_line = NULL;
            memset(&jobs_arr[i], 0, sizeof(jobs_arr[i]));
        }
    }
    jobs_num = 0;
}
