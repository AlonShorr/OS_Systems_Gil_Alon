//jobs.h

#ifndef JOBS_H
#define JOBS_H

/*=============================================================================
* includes, defines, usings
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "commands.h"
#include "signals.h"

#define ERROR -1
#define MAX_JOBS 100
#define JOB_RUNNING_FG 0
#define JOB_RUNNING_BG 1
#define JOB_STOPPED 2

/*=============================================================================
* global variables & data structures
=============================================================================*/

typedef struct {
    int job_id;
    int pid;
    char* cmd_line;
    time_t start_time;
    time_t seconds_elapsed;
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

/*=============================================================================
* Function Declarations
=============================================================================*/

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
int add_job(int pid, const char *cmd_line, int status);

/**
 * @brief Removes the job at the given array index (job_id).
 * job_id == array index --> it zeroes array[job_id] and decrement the active job counter. 
 * Leaving holes is intentional because each job keeps its job_id until it finishes.
 * @param job_id to remove
 */
void remove_job(int job_id);

int get_job_id_by_pid(int pid);

job *find_job_by_id(int job_id);

job *find_job_by_pid(int pid);

void update_job_status(int pid, int status);

void print_jobs();

void remove_finished_jobs();

void clean_all_jobs();


































#endif