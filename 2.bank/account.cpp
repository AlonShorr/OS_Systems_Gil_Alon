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
    init_rw_lock(&account_lock);
}
account::~account() {
    destroy_rw_lock(&account_lock);
}

int account::getId() const {return id;}
int account::getPassword() const {return password;}
void account::setPassword(int password) {this->password = password;}
void account::setBalance(double amount) {
    writer_lock(&account_lock);
    reader_lock(&account_lock);
    balance = amount;
    reader_unlock(&account_lock);
    writer_unlock(&account_lock);
}
double account::getBalance(){
    writer_lock(&account_lock);
    double b = balance;
    writer_unlock(&account_lock);
    return b;
}