#ifndef INCLUDES_H
#define INCLUDES_H

#include <iostream>      // for printing to the terminal (e.g., std::cout)
#include <fstream>       // for file operations (optional, since you're using system calls to write to log)
#include <string>        // for using std::string
#include <sstream>       // for string parsing (e.g., parsing ATM input lines)
#include <map>           // for storing accounts in Bank (e.g., std::map<int, Account*>)
#include <vector>        // for lists (e.g., snapshots, ATMs)
#include <unistd.h>      // for sleep(), usleep(), and system calls
#include <pthread.h>     // for threads and mutexes
#include <fcntl.h>       // for open() system call for writing logs
#include <sys/types.h>   // for types used in system calls
#include <sys/stat.h>    // for file modes
#include <cstring>       // for C-style string manipulation (e.g., strcmp)
#include <cstdlib>       // for atoi(), rand(), srand()
#include <ctime>         // for time(), for random seed and timestamps
#include <random>        // for random number generation

#define ARGS_NUM_MAX 10
#define ERROR 1
#define SUCCESS 0
//#include "bank.hpp"
//#include "account.h"
//#include "read_write_lock.hpp"


#endif // INCLUDES_H