// reader_writer_lock.h
#ifndef READE_WRITE_LOCK_H
#define READE_WRITE_LOCK_H

#include "includes.hpp"

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t readers_ok;
    pthread_cond_t writers_ok;
    int readers_inside;
    int writers_inside;
} rw_lock_t;

void init_rw_lock(rw_lock_t* rw);
void reader_lock(rw_lock_t* rw);
void reader_unlock(rw_lock_t* rw);
void writer_lock(rw_lock_t* rw);
void writer_unlock(rw_lock_t* rw);
void destroy_rw_lock(rw_lock_t* rw);

#endif