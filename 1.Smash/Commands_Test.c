#include <stdio.h>
#include "commands_test.h"

int showpid(){
    printf("showpid was called");
    return 0;
}
int pwd(){
    printf("pwd was called");
    return 0;
}
int cd(char* path){
    printf("cd was called");
    return 0;
}
int jobs(){
    printf("jobs was called");
    return 0;
}
int smash_kill(int signum, char* job_id){
    printf("smash_kill was called");
    return 0;
}
int fg(char* job_id){
    printf("fg was called");
    return 0;
}
int bg(char* job_id){
    printf("bg was called");
    return 0;
}
int diff(char* file1, char* file2){
    printf("diff was called");
    return 0;
}
int quit(){
    printf("quit was called");
    return 0;
}

void update_jobs(){
    printf("jobs array is updated");
}
 


