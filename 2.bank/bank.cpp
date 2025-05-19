#include "includes.hpp"
#include "bank.hpp"
#include "logger.hpp"


/*=============================================================================
    Helper Functions
=============================================================================*/
bool account_exists(const vector<account>& accounts, int account_id) {
    for (size_t i = 0; i < accounts.size(); ++i) {
        if (accounts[i].getId() == account_id) {
            return true;
        }
    }
    return false;
}

    /*=============================================================================
        bank Class Methods & others
    =============================================================================*/

    static rw_lock_t bank_lock;

    // create account class - need to kick to another file + fix functions
    account() {
        pthread_mutex_init(&lock, nullptr);
    }

    ~account() {
        pthread_mutex_destroy(&lock);
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
    
    //lock cuz atomic + loop to lock all accounts
    writer_lock(&bank_lock);   
    for (auto& acc : accounts) {
        pthread_mutex_lock(&acc.account_lock);
    }

    //write to log all taxes
    for (const auto& acc : accounts) {
        need_to_pay = acc.balance * tax_precent / 100;
        acc.balance -= need_to_pay;
        total_balance += need_to_pay;
        sprintf(buffer, "Bank: commissions of %d %% were charged, bank gained %f from account %d",tax_precent, need_to_pay, acc.id);
        write_log(std::string(buffer));
        withdraw(acc.id, acc.password, need_to_pay);
    }
     
    //unlock cuz atomic + unlock bank for adding/deleting accounts
    for (auto& acc : accounts) {
        pthread_mutex_unlock(&acc.account_lock);
    }
    writer_unlock(&bank_lock);
}

void BANK::print_accounts(){

    printf("\033[2J");
    printf("\033[1;1H");


    //lock so no one will add/delete acc + loop to lock all accounts
    writer_lock(&bank_lock);   
    for (auto& acc : accounts) {
        pthread_mutex_lock(&acc.account_lock);
    }

    //sort accounts by id - make a copy of our accounts vector
    std::vector<Account*> sorted_accounts = accounts;
    std::sort(sorted_accounts.begin(), sorted_accounts.end(), [](Account* a, Account* b) {return a->id < b->id;});
    //print all accounts
    std::cout << "Current Bank Status\n";
    for (const auto acc : sorted_accounts) {
        std::cout << "Account " << acc.id << ": Balance - " << acc.balance << "$, Account Password - " << acc.password << "\n";
    }
    //loop to unlock all accounts + unlock bank for adding/deleting accounts
    for (auto& acc : accounts) {
        pthread_mutex_unlock(&acc.account_lock);
    }
    writer_unlock(&bank_lock);
}

//ALON===========================================
int bank::getAccount_index(int id) {
    for (size_t i = 0; i < accounts.size(); ++i) {
        if (accounts[i].getId() == id) {
            return i;
        }
    }
    return -1;
}
//TODO: to implement!
/**
 * @brief: Bank functions to manage accounts
 * @return: 0 on success, 1 on failure
 */
 
int bank::open_new_account(int account_id, int password, double initial_balance, int atm_id) {
    ostringstream oss;
    if(account_id <= 0 || password <= 0 || initial_balance < 0) 
        return ERROR;
    else if(account_exists(this->accounts, account_id)) {
        oss << "Error " << atm_id << ": Your transaction failed - account with the same id exists";
        write_log(oss.str());
        return ERROR;
    }
    this->accounts.push_back(account(account_id, password, initial_balance)); 
    oss << atm_id << ": New account id is " << account_id
    << " with password " << password
    << " and initial balance " << initial_balance;
    write_log(oss.str());
    return SUCCESS;
}


int bank::deposit(int account_id, int account_password, double amount, int atm_id){
    
    if(account_id <= 0 || account_password <= 0 || amount < 0)
        return ERROR;

    int index = getAccount_index(account_id);
    ostringstream oss;

    if(accounts[index].getPassword() != account_password){
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }
    double newBalance = accounts[index].getBalance() + amount;
    accounts[index].setBalance(newBalance);
    oss << atm_id << ": Account " << account_id
    << " new balance is " << newBalance
    << " after " << amount << " $ was deposited";
    write_log(oss.str());
    return SUCCESS;
}

//int withdraw(int account_id, double amount);    
//int check_balance(int account_id); 
//int close_account(int account_id, int password); 
//int transfer(int from_account_id, int to_account_id, double amount);
//int close_atm(int id); 

int main (int argc, char *argv[]) {
    const int atm_num = argc - 2;
    std::vector<FILE*> input_files(atm_num);
    init_log("log.txt");

    // 1) Loop through input files
    for (int i = 1; i < atm_num + 1; i++) {
        input_files[i] = fopen(argv[i], "r");
        if (input_files[i] == nullptr) {
            std::cerr << "Error opening file: " << argv[i + 1] << std::endl; //TODO: remove before submission
            close_log();
            for (int j = 0; j < i; ++j) {
                fclose(input_files[j]);
            }
            return ERROR; 
        }
    }

    // 2) Create the main bank object, ATM list and threads
    bank* main_bank = new bank();
    std::vector<ATM *> ATMs(atm_num);

    for (int i = 0; i < atm_num; i++) {
        ATMs[i] = new ATM(i + 1, input_files[i], main_bank);
        ATMs[i]->start(); //starts and runs the ATM thread
    }

    // 3) Wait for all ATM threads to finish
    for (int i = 0; i < atm_num; i++) {
        ATMs[i]->join();
    }

    // 4) Clean up memory and close files
    delete main_bank;
    for (int i = 0; i < atm_num; ++i) {
        delete ATMs[i]; // the files are closed in the d'tor
    }

    close_log();

    
    return 0;
}

