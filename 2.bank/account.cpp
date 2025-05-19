#include "includes.hpp"
#include "account.hpp"
using namespace std;

/*=============================================================================
    Helper Functions
=============================================================================*/


/*=============================================================================
    Account Class Methods
=============================================================================*/

account::account(int id = 0, int password = 0, double balance = 0) : 
    id(id), password(password), balance(balance) {
    pthread_mutex_init(&account_lock, nullptr);
}
account::~account() {
    pthread_mutex_destroy(&account_lock);
}

int account::getId() const {return id;}
int account::getPassword() const {return password;}
void account::setPassword(int password) {this->password = password;}
void account::setBalance(double amount) {
    pthread_mutex_lock(&account_lock);
    balance = amount;
    pthread_mutex_unlock(&account_lock);
}
double account::getBalance(){
    pthread_mutex_lock(&account_lock);
    double b = balance;
    pthread_mutex_unlock(&account_lock);
    return b;
}