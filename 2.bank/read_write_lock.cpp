#include "includes.hpp"
#include "read_write_lock.hpp"
using namespace std;
extern int cm;

void init_rw_lock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_init(&rw->lock, nullptr);
    if(res!= 0) {
        errno = res;
        perror("Bank error1");
        return;
    }
    res = pthread_cond_init(&rw->readers_ok, nullptr);
    if(res!= 0) {
        errno = res;
        perror("Bank error2");
        return;
    }
    res = pthread_cond_init(&rw->writers_ok, nullptr);
    if(res!= 0) {
        errno = res;
        perror("Bank error3");
        return;
    }
    rw->readers_inside = 0;
    rw->writers_inside = 0;
    if(cm) {std::cout << "Initialized rw_lock " << rw << std::endl;}
}

void reader_lock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_lock(&rw->lock);
    if(res!= 0){
        errno = res;
        perror("Bank error4");
        return;
    }
    if(cm) {std::cout << "[Reader] waiting. Writers inside: " << rw->writers_inside << std::endl;}
    while (rw->writers_inside > 0) {
        res = pthread_cond_wait(&rw->readers_ok, &rw->lock);
        if(res!= 0){
            errno = res;
            perror("Bank error5");
            pthread_mutex_unlock(&rw->lock); // <-- Added unlock before return
            return;
        }
    }
    rw->readers_inside++;
    if(cm) std::cout << "[Reader] acquired. Readers inside: " << rw->readers_inside << std::endl;
    res = pthread_mutex_unlock(&rw->lock);
    if(res!= 0){
        errno = res;
        perror("Bank error6");
        return;
    }
}

void reader_unlock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_lock(&rw->lock);
    if(res!= 0){
        errno = res;
        perror("Bank error7");
        return;
    }
    rw->readers_inside--;
    if (rw->readers_inside == 0) {
        res = pthread_cond_signal(&rw->writers_ok);
        if(res!= 0){
            errno = res;
            perror("Bank error8");
            pthread_mutex_unlock(&rw->lock); // <-- Added unlock before return
            return;
        }
    }
    res = pthread_mutex_unlock(&rw->lock);
    if(res!= 0){
        errno = res;
        perror("Bank error9");
        return;
    }
}

void writer_lock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_lock(&rw->lock);
    if(res!= 0){
        errno = res;
        perror("Bank error10");
        return;
    }
    while (rw->writers_inside > 0 || rw->readers_inside > 0) {
        res = pthread_cond_wait(&rw->writers_ok, &rw->lock);
        if(res!= 0){
            errno = res;
            perror("Bank error11");
            pthread_mutex_unlock(&rw->lock); // <-- Added unlock before return
            return;
        }   
    }
    rw->writers_inside++;
    res = pthread_mutex_unlock(&rw->lock);
    if(res!= 0){
        errno = res;
        perror("Bank error12");
        return;
    }  
}

void writer_unlock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_lock(&rw->lock);
    if(res!= 0){
        errno = res;
        perror("Bank error13");
        return;
    }  
    rw->writers_inside--;
    res =  pthread_cond_broadcast(&rw->readers_ok);
    if(res!= 0){
        errno = res;
        perror("Bank error14");
        pthread_mutex_unlock(&rw->lock); // <-- Added unlock before return
        return;
    }  
    res = pthread_cond_signal(&rw->writers_ok);
    if(res!= 0){
        errno = res;
        perror("Bank error15");
        pthread_mutex_unlock(&rw->lock); // <-- Added unlock before return
        return;
    }  
    res = pthread_mutex_unlock(&rw->lock);
    if(res!= 0){
        errno = res;
        perror("Bank error16");
        return;
    }   
}

void destroy_rw_lock(rw_lock_t* rw) {
    int res;
    res = pthread_mutex_destroy(&rw->lock);
    if(res!= 0){
        errno = res;
        perror("Bank error17");
        return;
    }   
    res = pthread_cond_destroy(&rw->readers_ok);
    if(res!= 0){
        errno = res;
        perror("Bank error18");
        return;
    }  
    res = pthread_cond_destroy(&rw->writers_ok);
    if(res!= 0){
        errno = res;
        perror("Bank error19");
        return;
    }  
}