#ifndef BANK_H
#define BANK_H

#include "atm.hpp"
#include "includes.hpp"
using namespace std;

class account {
    private:
        int id;                            // Account ID
        int password;                      // Account password
        double balance;                    // Account balance
        pthread_mutex_t account_lock;     // Mutex for thread safety

    public:
        accout();
        account(int id, double balance);    // Constructor
        ~account();                         // Destructor
        int getId();                       // Get account ID
        int getPassword();                 // Get account password
        void setPassword(int password);    // Set account password
        double getBalance();                // Get balance
        void setBalance(double amount);    // Set balance
};

class bank {
    private:
        std::vector<account> accounts;   // Indexed by account ID
        std::vector<ATM> ATMs;           // List of ATMs
        pthread_mutex_t bank_lock;       // Protects account creation/deletion
        double total_balance;          // Total balance in the bank

    public:
        bank();
        ~bank();
    
        account* getAccount(int id);
        //bool createAccount(int id, int password, int balance);
        //bool deleteAccount(int id, int password);
        
        int get_random(int min, int max);   // Get a random percentage
        void tax();                         //tax the accounts
        void print_accounts();              // Print all accounts
 
        
        /**
         * @brief: Bank functions to manage accounts
         * @return: 0 on success, 1 on failure
         */
        int open_new_account(int account_id, int password, double initial_balance); 
        int deposit(int account_id, double amount);   
        int withdraw(int account_id, double amount);    
        int check_balance(int account_id); 
        int close_account(int account_id, int password); 
        int transfer(int from_account_id, int to_account_id, double amount);
        int close_atm(int id); 
        
};
    
#endif // BANK_H
