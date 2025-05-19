#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "includes.hpp"
#include "read_write_lock.hpp"
using namespace std;

class account {
    private:
        int id;                            // Account ID
        int password;                      // Account password
        double balance;                    // Account balance
        rw_lock_t account_lock;     // Mutex for thread safety

    public:
        account(int id, int password, double balance);    // Constructor
        ~account();                         // Destructor
        int getId() const;                       // Get account ID
        int getPassword() const;                 // Get account password
        void setPassword(int password);    // Set account password
        double getBalance();                // Get balance
        void setBalance(double amount);    // Set balance
};

#endif // ACCOUNT_H