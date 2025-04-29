// signals.c
#include "signals.h"



//required functions for signal handling

void ctrl_c_handler(int sig) {  //perhaps with arg int sig
    printf("smash: caught CTRL+C\n"); //perhaps after if condition
   // int fg_index = find_foreground_job();  // <-- function that returns index of foreground job, or -1 if none
    if (fg_pid != showpid()) {
        if (kill(-fg_pid, SIGKILL) == -1) {
            perror("smash error: kill(SIGKILL)");// will not happen - if we see this print just delete the print line
        }
        else{
            printf("smash: process %d was killed\n", fg_pid); 
        }
    }
}

void ctrl_z_handler(int sig) {  //perhaps with arg int sig
    printf("smash: caught CTRL+Z\n"); //perhaps after if condition
   // int fg_index = find_foreground_job();
    if (fg_pid != showpid()) {
        if (kill(-fg_pid, SIGTSTP) == -1) {
            perror("smash error: kill(SIGTSTP)");
        } else {
            add_job(fg_pid, fg_cmd_line, JOB_STOPPED);
            printf("smash: process %d was stopped\n", fg_pid);
        }
    }
}//===================================================================

//helper function to find foreground job in the jobs array
int find_foreground_job() {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_arr[i].pid > 0 && jobs_arr[i].status == JOB_RUNNUNG_FG) {
            return i;
        }
    }
    return -1; // No foreground job
}
