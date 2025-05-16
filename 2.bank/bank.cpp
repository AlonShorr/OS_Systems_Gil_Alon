#include "includes.hpp"
#include "bank.hpp"
#include "logger.hpp"



static rw_lock_t bank_lock;



// create account class - need to kick to another file + fix functions
    account() {
        pthread_mutex_init(&account_lock, nullptr);
    }

    ~account() {
        pthread_mutex_destroy(&account_lock);
    }
//-----------------------------------------------------


void BANK::RUN(){
    total_balance = 0;
    init_rw_lock(&bank_lock);


    //run bank as a thread, with loops for timing tax+printing




    destroy_rw_lock(&bank_lock);
}




int BANK::get_random(int low, int high) {
    static std::random_device rd;   // Seed generator
    static std::mt19937 gen(rd());  // Mersenne Twister RNG
    std::uniform_int_distribution<> dist(low, high);
    return dist(gen);
}

void BANK::tax(){
    
    double need_to_pay = 0;
    int tax_precent = get_random(1, 5);
    //lock if atomic
   
    
    //sort accounts by id - make a copy of our accounts vector, DELETE IF NO NEED TO WRITE IN ORDER
    std::vector<Account*> sorted_accounts = accounts;
    std::sort(sorted_accounts.begin(), sorted_accounts.end(), [](Account* a, Account* b) {
        return a->id < b->id;
    });
    
    //write to log all taxes
    for (const auto acc : sorted_accounts) {     //if not need sorted use:  for (const auto& acc : accounts) {
        pthread_mutex_lock(&acc.account_lock); //delete if tax is atomic - because i will lock before
        need_to_pay = acc.balance * tax_precent / 100;
        acc.balance -= need_to_pay;
        total_balance += need_to_pay;
        sprintf(buffer, "Bank: commissions of %d %% were charged, bank gained %f from account %d",tax_precent, need_to_pay, acc.id);
        write_log(std::string(buffer));
        pthread_mutex_unlock(&acc.account_lock); //delete if tax is atomic - because i will lock before
    }
     
    //unlock if atomic
 
}

void BANK::print_accounts(){

    printf("\033[2J");
    printf("\033[1;1H");

    //check if need to delet accounts





    //lock so no one will add/delete acc
    writer_lock(&bank_lock);   
    //loop to close all accounts
    for (auto& acc : accounts) {
        pthread_mutex_lock(&acc.account_lock);
    }
    //sort accounts by id - make a copy of our accounts vector
    std::vector<Account*> sorted_accounts = accounts;
    std::sort(sorted_accounts.begin(), sorted_accounts.end(), [](Account* a, Account* b) {
        return a->id < b->id;
    });
    //print all accounts
    std::cout << "Current Bank Status\n";
    for (const auto acc : sorted_accounts) {
        std::cout << "Account " << acc.id 
                  << ": Balance - " << acc.balance <<
                  << "$, Account Password - " << acc.password << "\n";
    }
    //loop to open all accounts
    for (auto& acc : accounts) {
        pthread_mutex_unlock(&acc.account_lock);
    }
    //open bank for adding/deleting accounts
    writer_unlock(&bank_lock);
}