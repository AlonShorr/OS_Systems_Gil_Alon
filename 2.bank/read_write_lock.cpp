#include "includes.hpp"
#include "read_write_lock.hpp"
using namespace std;
extern int cm;

void init_rw_lock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_init(&rw->lock, nullptr);
    if(res!= 0) {
        cerr <<"Bank error: " << res << "failed";
        return;
    }
    res = pthread_cond_init(&rw->readers_ok, nullptr);
    if(res!= 0) {
        cerr <<"Bank error: " << res << "failed";
        return;
    }
    pthread_cond_init(&rw->writers_ok, nullptr);
    if(res!= 0) {
        cerr <<"Bank error: " << res << "failed";
        return;
    }
    rw->readers_inside = 0;
    rw->writers_inside = 0;
}

void reader_lock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_lock(&rw->lock);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }
    std::cout << "[Reader] waiting. Writers inside: " << rw->writers_inside << std::endl;
    while (rw->writers_inside > 0) {
        res = pthread_cond_wait(&rw->readers_ok, &rw->lock);
        if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
        }
    }
    rw->readers_inside++;
    std::cout << "[Reader] acquired. Readers inside: " << rw->readers_inside << std::endl;
    res = pthread_mutex_unlock(&rw->lock);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }
}

void reader_unlock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_lock(&rw->lock);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }
    rw->readers_inside--;
    if (rw->readers_inside == 0) {
        res = pthread_cond_signal(&rw->writers_ok);
        if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
        }
    }
    res = pthread_mutex_unlock(&rw->lock);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }
}

void writer_lock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_lock(&rw->lock);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }
    while (rw->writers_inside > 0 || rw->readers_inside > 0) {
        res = pthread_cond_wait(&rw->writers_ok, &rw->lock);
        if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
        }   
    }
    rw->writers_inside++;
    res = pthread_mutex_unlock(&rw->lock);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }  
    
}

void writer_unlock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_lock(&rw->lock);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }  
    rw->writers_inside--;
    res =  pthread_cond_broadcast(&rw->readers_ok);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }  
    res = pthread_cond_signal(&rw->writers_ok);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }  
    res = pthread_mutex_unlock(&rw->lock);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }   
}

void destroy_rw_lock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_destroy(&rw->lock);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }   
    res = pthread_cond_destroy(&rw->readers_ok);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }  
    res = pthread_cond_destroy(&rw->writers_ok);
    if(res!= 0){
        cerr <<"Bank error: " << res << "failed";
        return;
    }  
}