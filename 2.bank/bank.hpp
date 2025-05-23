#ifndef BANK_H
#define BANK_H
#pragma once

#include "includes.hpp"
#include "account.hpp"
using namespace std;
//global variables
class ATM;
extern int thread_counter;
extern double global_balance;
extern int cm; //for debugging - if 1 we print everything to the screen

class bank {
    private:
        double total_balance;   // Total balance in the bank
    public:
        vector<account> accounts;   // Indexed by account ID
        vector<ATM> ATMs;           // List of ATMs
        rw_lock_t bank_lock;       // Protects account creation/deletion
        bank();
        ~bank();
    
        int getAccount_index(int id);
        //bool createAccount(int id, int password, int balance);
        //bool deleteAccount(int id, int password);
        
        int get_random(int min, int max);   // Get a random percentage
        void tax();                         //tax the accounts
        void print_accounts();              // Print all accounts
        account& get_account(int id);

        /**
         * @brief: Bank functions to manage accounts
         * @return: 0 on success, 1 on failure
         */
        void set_bank_balance(double amount);
        double get_bank_balance();
        int open_new_account(int account_id, int password, double initial_balance, int);
        int deposit(int account_id, int account_password, double amount, int atm_id);
        int withdraw_tax(int account_id, int password, double need_to_pay);
        int withdraw(int account_id, int password, double amount, int atm_id);
        int check_balance(int account_id, int password, int atm_id);
        int close_account(int account_id, int password, int atm_id); 
        int transfer(int from_account_id, int password, int to_account_id, double amount, int atm_id);
        int close_atm(int target_atm_id, int killer_atm_id);
        
};

#endif // BANK_H
