#ifndef LOGGER_H
#define LOGGER_H
//#include "includes.hpp"
#include "includes.hpp"
#include "read_write_lock.hpp"  // your module


//static rw_lock_t log_lock;
//static FILE* log_file = nullptr;

void init_log(const std::string& filename);
void write_log(const std::string& message);
void close_log();


#endif // LOGGER_H