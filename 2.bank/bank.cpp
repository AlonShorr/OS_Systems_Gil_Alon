#include "includes.hpp"
#include "bank.hpp"
#include "logger.hpp"
#include "atm.hpp"
#include "account.hpp"
using namespace std;
int thread_counter = 0;
double global_balance = 0;

int cm = 1; //for debugging - if 1 we print everything to the screen
/*=============================================================================
    Helper Functions
=============================================================================*/
int bank::getAccount_index(int id) {
    for (size_t i = 0; i < accounts.size(); ++i) {
        writer_lock(&accounts[i].account_lock);
        if (accounts[i].getId() == id) {
            return i;  //we unlock in function
        }
        else{
            writer_unlock(&accounts[i].account_lock);
        }
    }
    return -1;
}

int bank::get_random(int low, int high) {
    static std::random_device rd;   // Seed generator
    static std::mt19937 gen(rd());  // Mersenne Twister RNG
    std::uniform_int_distribution<> dist(low, high);
    return dist(gen);
}
/*=============================================================================
    bank Class Methods & others
=============================================================================*/
bank::bank() {
    this->total_balance = 0;
    init_rw_lock(&bank_lock);
}

bank::~bank() {
    destroy_rw_lock(&bank_lock); 
}

void bank::set_bank_balance(double amount) {total_balance = amount;} //lock and unlock outside the func
double bank::get_bank_balance(){return total_balance;} //lock and unlock outside the func

void bank::tax() {  
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
    ostringstream oss;
    for (auto& acc : accounts) {
        need_to_pay = acc.balance * tax_precent / 100;
        global_balance += need_to_pay;
        acc.balance -= need_to_pay;
        //withdraw_tax(acc.id, acc.password, need_to_pay);
        oss << "Bank: commissions of " << tax_precent << " % were charged, bank gained "<< need_to_pay <<" from account "<< acc.id;
        write_log(oss.str()); //must be inside critical section - tax is atomic and we need to write so much different logs. we cant contain them all in different place. it will be fast tho, because everything is locked so the log write will be very fast.
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

    if (cm) {printf("in print_accounts\n");}
    //lock so no one will add/delete acc + loop to lock all accounts, we dont care about readers - there are no changes in data
    writer_lock(&bank_lock);   
    for (auto& acc : accounts) {
        writer_lock(&acc.account_lock);
    }
    //sort accounts by id - make a copy of our accounts vector
    //std::vector<account*> sorted_accounts = accounts;
    //std::sort(sorted_accounts.begin(), sorted_accounts.end(), [](account* a, account* b) {return a->id < b->id;});
    if (cm) {printf("in print_accounts befor sort\n");}

    std::vector<account> sorted_accounts = accounts;
    std::sort(sorted_accounts.begin(), sorted_accounts.end(), [](const account& a, const account& b) {
        return a.id < b.id;
    });
    if (cm) {printf("in print_accounts after sort\n");}
    if (cm) {std::cout << "accounts size: " << accounts.size() << std::endl;}

    //print all accounts
    std::cout << "Current Bank Status\n";
    for (auto& acc : sorted_accounts) {
        if (cm) {printf("in print_accounts wanting to print\n");}

        std::cout << "Account " << acc.id << ": Balance - " << acc.balance << "$, Account Password - " << acc.password << "\n";
    }
    if (cm) {printf("in print_accounts finished printing\n");}

    reader_lock(&bank_lock); //now we do care, we dont want readerd to reas while half deleted
    for (auto& acc : accounts) {
        reader_lock(&acc.account_lock);
    }
    if (cm) {printf("in print_accounts want to close atms\n");}

    for (auto it = ATMs.begin(); it != ATMs.end(); ) {
        if (it->closed) { // ATM is marked closed
            if (cm) {printf("in print_accounts found atm to close %d\n",it->id);}

            pthread_join(it->get_thread(), nullptr); // Wait for thread to finish
            //it = ATMs.erase(it); // Erase returns the next valid iterator
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
    if (cm) {printf("in print_accounts want to exit\n");}

}

int bank::open_new_account(int account_id, int password, double initial_balance, int atm_id) {
    if (cm) {printf("open_new_acc\n");}

    ostringstream oss;
    int index;
    if((index = getAccount_index(account_id)) != -1) {
        writer_unlock(&accounts[index].account_lock);
        oss << "Error " << atm_id << ": Your transaction failed - account with the same id exists";
        write_log(oss.str());
        return ERROR;
    }
    
    this->accounts.push_back(account(account_id, password, initial_balance)); 
    oss << atm_id << ": New account id is " << account_id
    << " with password " << password
    << " and initial balance " << initial_balance;
    writer_unlock(&accounts[index].account_lock);
    write_log(oss.str());
    return SUCCESS;
}

int bank::deposit(int account_id, int account_password, double amount, int atm_id) {
    if (cm) {printf("deposit\n");}

    ostringstream oss;
    int index = getAccount_index(account_id); // locked
    if(accounts[index].getPassword() != account_password){
        writer_unlock(&accounts[index].account_lock);
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }
    double newBalance = accounts[index].getBalance() + amount;
    if (cm) {printf("before lock\n");}
    reader_lock(&accounts[index].account_lock);
    accounts[index].setBalance(newBalance);
    if (cm) {printf("in lock\n");}
    reader_unlock(&accounts[index].account_lock);
    writer_unlock(&accounts[index].account_lock);
    if (cm) {printf("after lock\n");}

    oss << atm_id << ": Account " << account_id
    << " new balance is " << newBalance
    << " after " << amount << " $ was deposited";
    write_log(oss.str());
    return SUCCESS;
}

// int bank::withdraw_tax(int account_id, int password, double need_to_pay) {
//     int index = getAccount_index(account_id); //lock
//     if(accounts[index].getPassword() != password){
//         writer_unlock(&accounts[index].account_lock);
//         oss << "Error " << atm_id << ": Your transaction failed - password for account id "
//         << account_id << " is incorrect";
//         write_log(oss.str());
//         return ERROR;     
//     }
//     if(accounts[index].getBalance() < amount){
//         writer_unlock(&accounts[index].account_lock);
//         oss << "Error " << atm_id << ": Your transaction failed – account id "
//         << account_id << " balance is lower than " << amount;
//         write_log(oss.str());
//         return ERROR;
//     }
//     double newBalance = accounts[index].getBalance() - need_to_pay;
//     accounts[index].setBalance(newBalance);
//     write_log(oss.str());
//     return SUCCESS;
// } 


int bank::withdraw(int account_id, int password, double amount, int atm_id) {
    if (cm) {printf("withdraw\n");}
   
    ostringstream oss;
    int index = getAccount_index(account_id); //lock
    if(accounts[index].getPassword() != password){
        writer_unlock(&accounts[index].account_lock);
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;     
    }
    if(accounts[index].getBalance() < amount){
        writer_unlock(&accounts[index].account_lock);
        oss << "Error " << atm_id << ": Your transaction failed - account id "
        << account_id << " balance is lower than " << amount;
        write_log(oss.str());
        return ERROR;
    }
    double newBalance = accounts[index].getBalance() - amount;
    if(cm) printf ("before lock\n");
    reader_lock(&accounts[index].account_lock);
    accounts[index].setBalance(newBalance);
    if(cm) printf ("in lock\n");
    reader_unlock(&accounts[index].account_lock);
    writer_unlock(&accounts[index].account_lock);  
    if(cm) printf ("after lock\n");  
    oss << atm_id << ": Account " << account_id << " new balance is " 
    << newBalance << " after " << amount << " $ was withdrawn";
    write_log(oss.str());
    return SUCCESS;
} 

int bank::check_balance(int account_id, int password, int atm_id) {
    if (cm) {printf("check_balance\n");}

    ostringstream oss;
    int index = getAccount_index(account_id);
    if(accounts[index].getPassword() != password){
        writer_unlock(&accounts[index].account_lock);
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }
    double balance = accounts[index].getBalance();
    writer_unlock(&accounts[index].account_lock);
    oss << atm_id << ": Account " << account_id << " balance is " << balance;
    write_log(oss.str());
    return SUCCESS;
}

int bank::close_account(int account_id, int password, int atm_id) {
    if (cm) {printf("close account\n");}

    //in this function we MUST close everything - we are deleting accounts. because we use vector it shifts everything.
    //we cant afford both reading and writing to any account because all the indexes are gonna change.for example:
    //we dont want someone to get index to deposit from, when shift all acc, and then do a deposit - the lock will be on the wrong index

    ostringstream oss;
    int index = getAccount_index(account_id); //acc locked
    if(accounts[index].getPassword() != password){
        writer_unlock(&accounts[index].account_lock);
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }
    writer_unlock(&accounts[index].account_lock); //unlock to not be in deadlock later
    writer_lock(&bank_lock);   
    for (auto& acc : accounts) {
        writer_lock(&acc.account_lock);
        reader_lock(&acc.account_lock);
    }
    reader_lock(&bank_lock);    
    oss << atm_id << ": Account " << account_id << " is now closed. Balance was " << accounts[index].getBalance();
    accounts.erase(accounts.begin() + index);
    reader_unlock(&bank_lock);
    for (auto& acc : accounts) {
        reader_unlock(&acc.account_lock);
        writer_unlock(&acc.account_lock);
    }
    writer_unlock(&bank_lock);
    write_log(oss.str());
    return SUCCESS;
}

int bank::transfer(int from_account_id, int password, int to_account_id, double amount, int atm_id){
    if (cm) {printf("transfer\n");}
   
    ostringstream oss; 
    int from_index = getAccount_index(from_account_id); //lock account
    if(accounts[from_index].getPassword() != password){
        writer_unlock(&accounts[from_index].account_lock);
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << from_account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }
    int to_index = getAccount_index(to_account_id); //lock account
    if(accounts[from_index].getBalance() < amount){
        writer_unlock(&accounts[from_index].account_lock);
        writer_unlock(&accounts[to_index].account_lock);       
        oss << "Error " << atm_id << ": Your transaction failed - account id "
        << from_account_id << " is balance is lower than " << amount;
        write_log(oss.str());
        return ERROR;  
    }
    else{
        reader_lock(&accounts[from_index].account_lock);
        reader_lock(&accounts[to_index].account_lock);
        accounts[from_index].setBalance(accounts[from_index].getBalance()-amount);
        accounts[to_index].setBalance(accounts[to_index].getBalance()+amount);
        oss << atm_id << ": Transfer " << amount << " from account "
        << from_account_id << " to account " << to_account_id 
        << " new account balance is " << accounts[from_index].getBalance()
        << " new target account balance is " << accounts[to_index].getBalance(); //inside critical cuz using getBalance()
        reader_unlock(&accounts[from_index].account_lock);
        reader_unlock(&accounts[to_index].account_lock);
        writer_unlock(&accounts[from_index].account_lock);
        writer_unlock(&accounts[to_index].account_lock);
        write_log(oss.str());
        return SUCCESS; 
    }
    return SUCCESS; 
}

int bank::close_atm(int target_atm_id, int killer_atm_id){
    if (cm) {printf("close atm\n");}

    ostringstream oss;
    int is_in = 0;
    for (auto it = ATMs.begin(); it != ATMs.end(); ) {
        if (it->id == target_atm_id) { // ATM is marked closed
            if (it->closed == true) {
                oss << "Error " << killer_atm_id << ": Your close operation failed - ATM ID "
                << target_atm_id << " is already in a closed state";
                is_in = 2;
            }
            else {
                it->closed = true;
                is_in = 1;
            }
        }
    }
    if (is_in == 0){
        oss << "Error " << killer_atm_id << ": Your transaction failed - ATM ID " << target_atm_id << " does not exist";
    }
    else if (is_in == 1){
        oss << "Bank: ATM " << killer_atm_id << " closed " << target_atm_id << " successfully";
    }
    write_log(oss.str());
    return SUCCESS; 
}



int main (int argc, char *argv[]) {
    if (cm) {printf("starting...\n");}
    // 0) Check inputs
    if(argc < 2) {
        cerr << "Bank error: illegal arguments" << endl;
        return ERROR;
    }
    if (cm) {printf("more than 2 args\n");}

    const int atm_num = argc - 1;
    std::vector<FILE*> input_files(atm_num);
    init_log("log.txt");
    if (cm) {printf("created log, atm num is %d\n", atm_num);}

    // 1) Loop through input files
    for (int i = 0; i < atm_num; i++) {
        if (cm) {printf("looping through files\n");}
        // Open the input file for each ATM
        input_files[i] = fopen(argv[i+1], "r");
        if (input_files[i] == nullptr) {
            cerr << "Bank error: illegal arguments" << endl; 
            close_log();
            for (int j = 0; j < i; ++j) {
                fclose(input_files[j]);
            }
            return ERROR; 
        }
        if(cm) printf ("opened file: %d\n", i+1);
    }
    if (cm) {printf("after 1st loop\n");}

    // 2) Create the main bank object, ATM list and threads
    bank* main_bank = new bank();
    
    //put inside constractor:
        //std::vector<ATM *> ATMs(atm_num);
        //init_rw_lock(&bank_lock);
    
    if (cm) {printf("created bank\n");}

    for (int i = 0; i < atm_num; i++) {
        if (cm) {printf("created ATM\n");}
        main_bank->ATMs.emplace_back(i + 1, input_files[i], main_bank, false); // construct in-place
        main_bank->ATMs[i].start(); // call start on the constructed object
    }
    if (cm) {printf("inisialized bank\n");}

    //2.5) start timer
    auto start_time = std::chrono::steady_clock::now();
    auto last_tax_time = start_time;
    auto last_print_time = start_time;
    if (cm) {printf("started timer\n");}

    // 3) Wait for all ATM threads to finish, while taxing and printing accounts
    while (true) {
        auto now = std::chrono::steady_clock::now();

    
        //if (cm) {printf("started while\n");}

        // Check if 0.5s passed since last print
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_print_time).count() >= 500) {
            if (cm) {printf("in print\n");}
            last_print_time = now;
            main_bank->print_accounts();
        }
        //if (cm) {printf("after print\n");}

        // Check if 3s passed since last tax
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_tax_time).count() >= 3) {
            if (cm) {printf("in tax\n");}
            last_tax_time = now;
            main_bank->tax();
            main_bank->set_bank_balance(global_balance);
        }
        //if (cm) {printf("after tax\n");}

        if (thread_counter == atm_num){
            for (int i = 0; i < atm_num; i++) {
                main_bank->ATMs[i].join();
            }
            break;
        }
        // PERHAPS DELETE - tiny sleep to avoid CPU spinning at 100%
        usleep(1000);
    }
    if (cm) {printf("after while\n");}


    // 4) Clean up memory and close files
    
    delete main_bank;
    //inside destructor:    
        //for (int i = 0; i < atm_num; ++i) {
        //    delete ATMs[i]; // the files are closed in the d'tor
        //}
        //destroy_rw_lock(&bank_lock);
    
    close_log();
    if (cm) {printf("ending...\n");}

    return 0;
}

