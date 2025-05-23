#include "includes.hpp"
#include "logger.hpp"

extern int cm;


// One global log lock for the log file
static rw_lock_t log_lock;
static FILE* log_file;
//int cm = 1; //for debugging - if 1 we print everything to the screen

void init_log(const std::string& filename) {
    log_file = fopen(filename.c_str(), "w");
    if (!log_file) {
        perror("Failed to open log file");
        exit(1); // or handle gracefully
    }
    init_rw_lock(&log_lock);  
}

void write_log(const std::string& message) {
    if (cm) {std::cout << "LOGGING\n";}
    writer_lock(&log_lock);   
    fprintf(log_file, "%s\n", message.c_str());
    fflush(log_file);
    writer_unlock(&log_lock);
}

void close_log() {
    if (log_file) {
        fclose(log_file);
        log_file = nullptr;  
    }
    destroy_rw_lock(&log_lock);
}