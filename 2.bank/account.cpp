#include "includes.hpp"
#include "account.hpp"
using namespace std;
extern int cm;

/*=============================================================================
    Helper Functions
=============================================================================*/


/*=============================================================================
    Account Class Methods
=============================================================================*/

account::account(int id = 0, int password = 0, double balance = 0) : 
    id(id), password(password), balance(balance), closed(false) {
    init_rw_lock(&account_lock);
}
account::~account() {
    destroy_rw_lock(&account_lock);
}

int account::getId() const {return id;}
int account::getPassword() const {return password;}
void account::setPassword(int password) {this->password = password;}
void account::setBalance(double amount) {balance = amount;} //lock and unlock outside the func
double account::getBalance(){return balance;} //lock and unlock outside the func
