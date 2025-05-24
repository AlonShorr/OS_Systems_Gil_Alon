#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "includes.hpp"
#include "read_write_lock.hpp"
using namespace std;

class account {
    public:
        int id;                            // Account ID
        int password;                      // Account password
        double balance;                    // Account balance
        bool closed;
        rw_lock_t account_lock;     // rw_lock for thread safety

        account(int id, int password, double balance);    // Constructor
        ~account();                         // Destructor
        int getId() const;                       // Get account ID
        int getPassword() const;                 // Get account password
        void setPassword(int password);    // Set account password
        double getBalance();                // Get balance
        void setBalance(double amount);    // Set balance
};

#endif // ACCOUNT_H