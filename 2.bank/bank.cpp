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

int BANK::get_random(int low, int high) {
    static std::random_device rd;   // Seed generator
    static std::mt19937 gen(rd());  // Mersenne Twister RNG
    std::uniform_int_distribution<> dist(low, high);
    return dist(gen);
}
/*=============================================================================
    bank Class Methods & others
=============================================================================*/
void BANK::tax(){  
    double need_to_pay = 0;
    int tax_precent = get_random(1, 5);
    
    //lock cuz atomic + loop to lock all accounts
    writer_lock(&bank_lock); // Lock for writing
    for (auto& acc : accounts) {
        writer_lock(&acc.account_lock);
        reader_lock(&acc.account_lock);
    }
    reader_lock(&bank_lock); // Lock for reading - we dont want people to read while we still taxing untill we taxed all accounts  
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
    reader_unlock(&bank_lock); // unLock for reading   
    for (auto& acc : accounts) {
        reader_unlock(&acc.account_lock);
        writer_unlock(&acc.account_lock);
    }
    writer_unlock(&bank_lock);
}

void bank::print_accounts(){

    printf("\033[2J");
    printf("\033[1;1H");
    //lock so no one will add/delete acc + loop to lock all accounts, we dont care about readers - there are no changes in data
    writer_lock(&bank_lock);   
    for (auto& acc : accounts) {
        writer_lock(&acc.account_lock);
    }
    //sort accounts by id - make a copy of our accounts vector
    std::vector<Account*> sorted_accounts = accounts;
    std::sort(sorted_accounts.begin(), sorted_accounts.end(), [](Account* a, Account* b) {return a->id < b->id;});
    //print all accounts
    std::cout << "Current Bank Status\n";
    for (const auto acc : sorted_accounts) {
        std::cout << "Account " << acc.id << ": Balance - " << acc.balance << "$, Account Password - " << acc.password << "\n";
    }
    reader_lock(&bank_lock); //now we do care, we dont want readerd to reas while half deleted
    for (auto& acc : accounts) {
        reader_lock(&acc.account_lock);
    }
    for (auto it = ATMs.begin(); it != ATMs.end(); ) {
        ATM* atm = *it;
        if (atm->is_closed()) { // Check if ATM is marked closed
            int ret = pthread_tryjoin_np(atm->get_thread(), nullptr); // Try to join without blocking
            if (ret == 0) {
                delete atm; // Join succeeded — thread is done
                it = ATMs.erase(it);
            }
            else{
                ++it; // Join not possible yet — thread still running
            }
        }
        else {
            ++it;
        }
    }
    reader_unlock(&bank_lock);
    //loop to unlock all accounts + unlock bank for adding/deleting accounts
    for (auto& acc : accounts) {
        reader_unlock(&acc.account_lock);
        writer_unlock(&acc.account_lock);
    }
    writer_unlock(&bank_lock);
}

int bank::getAccount_index(int id) {
    for (size_t i = 0; i < accounts.size(); ++i) {
        if (accounts[i].getId() == id) {
            return i;
        }
    }
    return -1;
}

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

int bank::withdraw(int account_id, int password, double amount, int atm_id) { 
    if(account_id <= 0 || password <= 0 || amount < 0)
        return ERROR;

    int index = getAccount_index(account_id);
    ostringstream oss;

    if(accounts[index].getPassword() != password){
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;     
    }

    if(accounts[index].getBalance() < amount){
        oss << "Error " << atm_id << ": Your transaction failed – account id "
        << account_id << " balance is lower than " << amount;
        write_log(oss.str());
        return ERROR;
    }

    double newBalance = accounts[index].getBalance() - amount;
    accounts[index].setBalance(newBalance);
    oss << atm_id << ": Account " << account_id << " new balance is " 
    << newBalance << " after " << amount << " $ was withdrawn";
    write_log(oss.str());
    return SUCCESS;
} 

int bank::check_balance(int account_id, int password, int atm_id) {
    if(account_id <= 0 || password <= 0)
        return ERROR;

    int index = getAccount_index(account_id);
    ostringstream oss;

    if(accounts[index].getPassword() != password){
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }

    double balance = accounts[index].getBalance();
    oss << atm_id << ": Account " << account_id << " balance is " << balance;
    write_log(oss.str());
    return SUCCESS;
}

int bank::close_account(int account_id, int password, int atm_id) {
    if(account_id <= 0 || password <= 0)
        return ERROR;

    int index = getAccount_index(account_id);
    ostringstream oss;

    if(accounts[index].getPassword() != password){
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }

    //TODO: to edit
    accounts.erase(accounts.begin() + index);
    oss << "Account " << account_id << " was closed";
    write_log(oss.str());
    return SUCCESS;
    // until here
}


//int transfer(int from_account_id, int to_account_id, double amount);
//int close_atm(int id); 
//int withdraw(int account_id, double amount);    
//int check_balance(int account_id); 
//int close_account(int account_id, int password); 










int transfer(int from_account_id, int password, int to_account_id, double amount, int atm_id){
    int from_index = getAccount_index(from_account_id);
    int to_index = getAccount_index(to_account_id);
    ostringstream oss;
    bool valid = true;
    int from_new_amount;
    int to_new_amount;
    if(accounts[from_index].getPassword() != password){
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << from_account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }
    writer_lock(&accounts[from_index].read_write_lock);
    reader_lock(&accounts[from_index].read_write_lock);
    writer_lock(&accounts[to_index].read_write_lock);
    reader_lock(&accounts[to_index].read_write_lock);
    if(accounts[from_index].getBalance() < amount){
        valid = false;
        oss << "Error " << atm_id << ": Your transaction failed - account id "
        << from_account_id << " is balance is lower than " << amount;
    }
    else{
        accounts[from_index].setBalance(accounts[from_index].getBalance()-amount);
        accounts[to_index].setBalance(accounts[to_index].getBalance()+amount);
        oss << atm_id << ": Transfer " << amount << " from account "
        << from_account_id << " to account " << to_account_id 
        << " new account balance is " << accounts[from_index].getBalance()
        << " new target account balance is " << accounts[to_index].getBalance();
    }  
    writer_unlock(&accounts[from_index].read_write_lock);
    reader_unlock(&accounts[from_index].read_write_lock);
    writer_unlock(&accounts[to_index].read_write_lock);
    reader_unlock(&accounts[to_index].read_write_lock);
    if (valid){
        write_log(oss.str());
        return SUCCESS; 
    }
    else{  
        write_log(oss.str());
        return ERROR;   
    }
}









//int close_atm(int id){}










int main (int argc, char *argv[]) {
    const int atm_num = argc - 2;
    std::vector<FILE*> input_files(atm_num);
    init_log("log.txt");
    init_rw_lock(&bank_lock);
    total_balance = 0;

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

    //2.5) start timer
    auto start_time = steady_clock::now();
    auto last_tax_time = start_time;
    auto last_print_time = start_time;

    // 3) Wait for all ATM threads to finish, while taxing and printing accounts
    while (true) {
        auto now = steady_clock::now();
        // Check if 0.5s passed since last print
        if (duration_cast<milliseconds>(now - last_print_time).count() >= 500) {
            print_accounts();
            last_print_time = now;
        }
        // Check if 3s passed since last tax
        if (duration_cast<seconds>(now - last_tax_time).count() >= 3) {
            tax();
            last_tax_time = now;
        }

        if (thread_counter == atm_num){
            for (int i = 0; i < atm_num; i++) {
                ATMs[i]->join();
            }
            break;
        }
        // PERHAPS DELETE - tiny sleep to avoid CPU spinning at 100%
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }


    // 4) Clean up memory and close files
    delete main_bank;
    for (int i = 0; i < atm_num; ++i) {
        delete ATMs[i]; // the files are closed in the d'tor
    }

    close_log();
    destroy_rw_lock(&bank_lock);

    return 0;
}

