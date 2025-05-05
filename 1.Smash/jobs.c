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
extern pid_t fg_pid;           // current foreground PID

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

int get_job_id (job job) {
    for(int i=0; i < MAX_JOBS; i++) {
        if (jobs_arr[i].pid == job.pid)  
            return i;
    }
    return ERROR;
}

int get_id (pid_t pid) {
    for(int i=0; i < MAX_JOBS; i++) {
        if (jobs_arr[i].pid == pid)  
            return i;
    }
    return ERROR;
}

pid_t get_pid(job job)
{
    if (job.job_id < 0 || job.job_id >= MAX_JOBS) 
        return ERROR;
    return job.pid;
}

char* get_cmd_line (job job) {
    if (job.job_id < 0 || job.job_id >= MAX_JOBS) 
        return NULL;
    else if (job.cmd_line == NULL) 
        return NULL;  

    return job.cmd_line;
}

time_t get_start_time (job job) {
    if (job.job_id < 0 || job.job_id >= MAX_JOBS) 
        return ERROR;
     
    return job.start_time;
}

int get_status (job job) {
    if (job.job_id < 0 || job.job_id >= MAX_JOBS) 
        return ERROR;
  
    return job.status;
}

void init_jobs() {
    for(int i = 0; i < MAX_JOBS; i++) {
        jobs_arr[i].pid = 0;
        jobs_arr[i].cmd_line = NULL;
        jobs_arr[i].start_time = 0;
        jobs_arr[i].status = JOB_RUNNING_FG;
    }
    // Initialize the global job count
    jobs_num = 0;
}


int add_job(pid_t pid, const char *cmd_line, int status) {
    if (pid <= 0 || cmd_line == NULL) {
        return ERROR;
    }
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
    jobs_arr[job_id].pid = 0;
    jobs_arr[job_id].cmd_line = NULL;
    jobs_arr[job_id].start_time = 0;
    jobs_arr[job_id].status = JOB_RUNNING_FG;

    jobs_num--;
}


void remove_job_by_pid(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_arr[i].pid != 0 && jobs_arr[i].pid == pid) {
            remove_job(i);
            break;
        }
    }
}

/*
void remove_job_by_pid(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_arr[i].pid != 0 && jobs_arr[i].pid == pid) {;
            jobs_arr[i].pid = 0;
            //
            printf("Removing job with pid remove_by_pid: %d at index %d\n", jobs_arr[i].pid, i);
            //
            break;
        }
    }
    
    jobs_num--;
} */

void print_job(int job_id) {
    if ((job_id < 0 || job_id >= MAX_JOBS) 
         || jobs_arr[job_id].pid == 0)
        return;

    job *job = &jobs_arr[job_id];
    long elapsed = (long)(time(NULL) - job->start_time);

    if(job->status == JOB_STOPPED)
        printf("[%d] %s: %d %ld secs (stopped)\n",job->job_id, 
                                                  job->cmd_line,
                                                  job->pid,
                                                  elapsed); 
    else if(job->status == JOB_RUNNING_BG)
        printf("[%d] %s: %d %ld secs\n",job->job_id, 
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
    pid_t pid;
    int status;
    for (int i = 0; i < MAX_JOBS; i++) {
        pid = jobs_arr[i].pid;
        if (pid == 0 || pid == fg_pid) continue; // No job found for this pid or it is the fg process

        //check system status for the process the job is represnting:
        pid_t res = waitpid(pid, &status, WNOHANG);
        //handle status only if the status has changed: (else continue)
        if(res > 0) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) { // Job has exited or was killed by signal - remove from list
                remove_job(i);
            }

            else if(WIFSTOPPED(status)) { // Job has stopped - update its status
                update_job_status(i, JOB_STOPPED); 
            }
            else if(WIFCONTINUED(status)) { // Job has continued - update its status
                update_job_status(i, JOB_RUNNING_BG); 
            }
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
