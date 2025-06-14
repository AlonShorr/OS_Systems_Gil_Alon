//jobs.h

#ifndef JOBS_H
#define JOBS_H

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <stdio.h>    
#include <stdlib.h>   
#include <string.h>
#include <signal.h>
#include <sys/wait.h> 
#include <sys/types.h>
#include <time.h> 
   
#define ERROR -1
#define MAX_JOBS 100
#define JOB_RUNNING_FG 0
#define JOB_RUNNING_BG 1
#define JOB_STOPPED 2
#define CMD_LENGTH_MAX 120

/*=============================================================================
* global variables & data structures
=============================================================================*/

typedef struct job {
    int job_id;
    pid_t pid;
    char* cmd_line;
    time_t start_time;
    int status;
}job;

/**
 * @param jobs_arr a program global array that holds the jobs queue. The array
 * indexes pid == 0 for an empty spot. For an index containing a job job_id == it's index  
 * in the array.
 */
extern job jobs_arr[MAX_JOBS];

/**
 * @param jobs_nums: global number of jobs currently processed.
 */
extern int jobs_num;
extern char _cmd[CMD_LENGTH_MAX];

/*=============================================================================
* Function Declarations
=============================================================================*/

int get_job_id (job job);
int get_id(pid_t pid);
pid_t get_pid(job job);
char* get_cmd_line(job job);
time_t get_start_time(job job);
int get_status(job job);
/**
 * @brief initializes jobs queue (implemented as an array).
 */
void init_jobs();

/**
 * @brief initializes new job inluding time stamps.
 * @param pid: System process PID.
 * @param cmd_line: cmd line NOT PARSED.
 * @param status: runnig fg / bg, stopped.
 * @return returns ERROR on fail, new slot index on success
 */
int add_job(pid_t pid, const char *cmd_line, int status);

/**
 * @brief Removes the job at the given array index (job_id).
 * job_id == array index --> it zeroes array[job_id] and decrement the active job counter. 
 * Leaving holes is intentional because each job keeps its job_id until it finishes.
 * @param job_id to remove
 */
void remove_job(int job_id);


/**
 * @brief Removes the job at the given its pid.
 * It zeroes array[job_id].pid and decrement the active job counter. 
 * Leaving holes is intentional because each job keeps its job_id until it finishes.
 * @param job_id to remove
 */
void remove_job_by_pid(pid_t pid);

/**
 * @brief: updates a single job status in the jobs array.
 * @param job_id: the job id to update.
 * @param status: the new status to set.
 */
void update_job_status(int job_id, int status);

/**
 * @brief: updates all jobs in the jobs array. if a job has finished it will be removed from the array.
 * if a job has stopped it will be updated to JOB_STOPPED, and if a job has continued it will be updated to JOB_RUNNING_BG.
 */
void update_jobs();

/**
 * @brief: prints a job in the format: [<job id>] <command>: <process id> <seconds elapsed>
 * @note: adds (stopped) to the end of the line if the job is stopped.
 * @param job_id: the job id to print.
 */
void print_job(int job_id);

/**
 * @brief: zeros the jobs array and sets the number of jobs to 0.
 * @note: frees the cmd_line of each job in the array since it was dynamically allocated.
 * @note: this function should be called when the program is exiting to free all the memory allocated for the jobs.
 */
void clean_all_jobs();

#endif