#include "includes.hpp"
#include "read_write_lock.hpp"


void init_rw_lock(rw_lock_t* rw) {
    pthread_mutex_init(&rw->lock, nullptr);
    pthread_cond_init(&rw->readers_ok, nullptr);
    pthread_cond_init(&rw->writers_ok, nullptr);
    rw->readers_inside = 0;
    rw->writers_inside = 0;
}

void reader_lock(rw_lock_t* rw) {
    pthread_mutex_lock(&rw->lock);
    while (rw->writers_inside > 0) {
        pthread_cond_wait(&rw->readers_ok, &rw->lock);
    }
    rw->readers_inside++;
    pthread_mutex_unlock(&rw->lock);
}

void reader_unlock(rw_lock_t* rw) {
    pthread_mutex_lock(&rw->lock);
    rw->readers_inside--;
    if (rw->readers_inside == 0)
        pthread_cond_signal(&rw->writers_ok);
    pthread_mutex_unlock(&rw->lock);
}

void writer_lock(rw_lock_t* rw) {
    pthread_mutex_lock(&rw->lock);
    while (rw->writers_inside > 0 || rw->readers_inside > 0) {
        pthread_cond_wait(&rw->writers_ok, &rw->lock);
    }
    rw->writers_inside++;
    pthread_mutex_unlock(&rw->lock);
}

void writer_unlock(rw_lock_t* rw) {
    pthread_mutex_lock(&rw->lock);
    rw->writers_inside--;
    pthread_cond_broadcast(&rw->readers_ok);
    pthread_cond_signal(&rw->writers_ok);
    pthread_mutex_unlock(&rw->lock);
}

void destroy_rw_lock(rw_lock_t* rw) {
    pthread_mutex_destroy(&rw->lock);
    pthread_cond_destroy(&rw->readers_ok);
    pthread_cond_destroy(&rw->writers_ok);
}