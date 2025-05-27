#include "includes.hpp"
#include "bank.hpp"
#include "logger.hpp"
#include "atm.hpp"
#include "account.hpp"
using namespace std;
int thread_counter = 0;
double global_balance = 0;

int cm = 0; //for debugging - if 1 we print everything to the screen
/*=============================================================================
    Helper Functions
=============================================================================*/
int bank::getAccount_index_read(int id) {
   for (size_t i = 0; i < accounts.size(); ++i) {
        reader_lock(&accounts[i].account_lock);
        if (accounts[i].getId() == id) {
            if (accounts[i].closed == false) {
                return i;  //we unlock in function
            }
        }
        reader_unlock(&accounts[i].account_lock);
    }
    return -1;
}

int bank::getAccount_index_write(int id) {
   for (size_t i = 0; i < accounts.size(); ++i) {
        writer_lock(&accounts[i].account_lock);
        if (accounts[i].getId() == id) {
            if (accounts[i].closed == false) {
                return i;  //we unlock in function
            }
        }
        writer_unlock(&accounts[i].account_lock); 
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
    //    reader_lock(&acc.account_lock);
    }
    //reader_lock(&bank_lock); // Lock for reading - we dont want people to read while we still taxing untill we taxed all accounts  
    //write to log all taxes
    //for (auto& acc : accounts) {
    for (int i=0; i< static_cast<int>(accounts.size()); i++) {
        if (accounts[i].closed == false) {
            ostringstream oss;

            need_to_pay = accounts[i].balance * tax_precent / 100;
            global_balance += need_to_pay;
            accounts[i].balance -= need_to_pay;
            //withdraw_tax(acc.id, acc.password, need_to_pay);
            oss << "Bank: commissions of " << tax_precent << " % were charged, bank gained "<< need_to_pay <<" from account "<< accounts[i].id;
            write_log(oss.str()); //must be inside critical section - tax is atomic and we need to write so much different logs. we cant contain them all in different place. it will be fast tho, because everything is locked so the log write will be very fast.
        }
    }
    //unlock cuz atomic + unlock bank for adding/deleting accounts
    //reader_unlock(&bank_lock); // unLock for reading   
    for (auto& acc : accounts) {
        //reader_unlock(&acc.account_lock);
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
        if (cm) {printf("in print_accounts locking acc %d\n", acc.id);}
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
        if (acc.closed == false) {
            //
            if (cm) {printf("in print_accounts acc is in status:  %d \n", acc.closed);}
            //
            std::cout << "Account " << acc.id << ": Balance - " << acc.balance << "$, Account Password - " << acc.password << "\n";
        } 
    }
    if (cm) {printf("in print_accounts finished printing\n");}

    //reader_lock(&bank_lock); //now we do care, we dont want readerd to read while half deleted
    //for (auto& acc : accounts) {
    //    reader_lock(&acc.account_lock);
    //}
    if (cm) {printf("in print_accounts want to close atms\n");}
    
    for (int i=0; i< static_cast<int>(ATMs.size()); i++) {
        if (ATMs[i]->wanted_to_close) { // ATM is marked closed
            ATMs[i]->closed = true;
            if (cm) {printf("in print_accounts found atm to close %d\n",ATMs[i]->id);}
            //int res =
            pthread_join(ATMs[i]->get_thread(), nullptr); // Wait for thread to finish
            //if (res != 0) {
            //    errno = res;
            //    perror("Bank error");
            //}
        }
    }

\
    //reader_unlock(&bank_lock);
    //loop to unlock all accounts + unlock bank for adding/deleting accounts
    for (auto& acc : accounts) {
    //    reader_unlock(&acc.account_lock);
        writer_unlock(&acc.account_lock);
    }
    writer_unlock(&bank_lock);
    if (cm) {printf("in print_accounts want to exit\n");}

}

int bank::open_new_account(int account_id, int password, double initial_balance, int atm_id) {
    if (cm) {printf("open_new_acc\n");}
    ostringstream oss;
    int index;
    writer_lock(&bank_lock); // Lock for writing
    if((index = getAccount_index_read(account_id)) != -1) {
        reader_unlock(&accounts[index].account_lock);
        writer_unlock(&bank_lock); 
        oss << "Error " << atm_id << ": Your transaction failed - account with the same id exists";
        write_log(oss.str());
        return ERROR;
    }
    
    this->accounts.push_back(account(account_id, password, initial_balance));
    writer_unlock(&bank_lock);
    if (cm) {printf("after push_back\n");}
    oss << atm_id << ": New account id is " << account_id
    << " with password " << password
    << " and initial balance " << initial_balance;
    //writer_unlock(&accounts[index].account_lock); DO NOT CHANGE
    write_log(oss.str());
    if (cm) {printf("after open_new_acc\n");}
    return SUCCESS;
}

int bank::deposit(int account_id, int account_password, double amount, int atm_id) {
    
    if (cm) {printf("deposit\n");}
    ostringstream oss;
    reader_lock(&bank_lock);    
    int index = getAccount_index_write(account_id); // locked
    reader_unlock(&bank_lock);

    if(index == -1) {
        printf("account not exists\n");
        return ERROR;
    }
    if(accounts[index].getPassword() != account_password){
        writer_unlock(&accounts[index].account_lock);
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }
    double newBalance = accounts[index].getBalance() + amount;
    if (cm) {printf("before lock\n");}
    //reader_lock(&accounts[index].account_lock);
    accounts[index].setBalance(newBalance);
    if (cm) {printf("in lock\n");}
    //reader_unlock(&accounts[index].account_lock);
    writer_unlock(&accounts[index].account_lock);
    if (cm) {printf("after lock\n");}

    oss << atm_id << ": Account " << account_id
    << " new balance is " << newBalance
    << " after " << amount << " $ was deposited";
    write_log(oss.str());
    return SUCCESS;
}


int bank::withdraw(int account_id, int password, double amount, int atm_id) {
    if (cm) {printf("withdraw\n");}
   
    ostringstream oss;
    reader_lock(&bank_lock);    
    int index = getAccount_index_write(account_id); //lock
    reader_unlock(&bank_lock);

    if(index == -1) {
        printf("account not exists\n");
        return ERROR;
    }
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
    //reader_lock(&accounts[index].account_lock);
    accounts[index].setBalance(newBalance);
    if(cm) printf ("in lock\n");
    //reader_unlock(&accounts[index].account_lock);
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

    reader_lock(&bank_lock);    
    int index = getAccount_index_read(account_id); //read lock
    reader_unlock(&bank_lock);


    if(index == -1) {
        printf("account not exists\n");
        return ERROR;
    }
    if(accounts[index].getPassword() != password){
        reader_unlock(&accounts[index].account_lock);
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }
    double balance = accounts[index].getBalance();
    //writer_unlock(&accounts[index].account_lock); change to reader_unlock
    oss << atm_id << ": Account " << account_id << " balance is " << balance;
    reader_unlock(&accounts[index].account_lock);

    write_log(oss.str());
    return SUCCESS;
}

int bank::close_account(int account_id, int password, int atm_id) {
    if (cm) {printf("close account\n");}

    //in this function we MUST close everything - we are deleting accounts. because we use vector it shifts everything.
    //we cant afford both reading and writing to any account because all the indexes are gonna change.for example:
    //we dont want someone to get index to deposit from, when shift all acc, and then do a deposit - the lock will be on the wrong index

    ostringstream oss;
    reader_lock(&bank_lock);    
    int index = getAccount_index_write(account_id); //acc locked
    reader_unlock(&bank_lock);

    if(index == -1) {
        printf("account not exists\n");
        return ERROR;
    }
    if(accounts[index].getPassword() != password){
        writer_unlock(&accounts[index].account_lock);
        oss << "Error " << atm_id << ": Your transaction failed - password for account id "
        << account_id << " is incorrect";
        write_log(oss.str());
        return ERROR;   
    }
    //writer_unlock(&accounts[index].account_lock); //unlock to not be in deadlock later
    //writer_lock(&bank_lock);
    //reader_lock(&bank_lock);    
    oss << atm_id << ": Account " << account_id << " is now closed. Balance was " << accounts[index].getBalance();
    accounts[index].closed = true;
    if(cm && accounts[index].closed) printf("account %d is closed", account_id); 
    //accounts.erase(accounts.begin() + index);
    writer_unlock(&accounts[index].account_lock);
    //writer_unlock(&bank_lock);
    write_log(oss.str());
    return SUCCESS;
}

int bank::transfer(int from_account_id, int password, int to_account_id, double amount, int atm_id){
    
    // WHAT IF TO AND FROM ARE THE SAME ACC?????
    
    if (cm) {printf("transfer\n");}
    ostringstream oss; 
    int from_index;
    int to_index;

    reader_lock(&bank_lock);    
    if (from_account_id > to_account_id) { // swap if needed in order to avoid deadlock
        to_index = getAccount_index_write(to_account_id); //lock account
        if(to_index == -1) {
            reader_unlock(&bank_lock);
            printf("Destination account not exists\n");
            return ERROR;
        }
        from_index = getAccount_index_write(from_account_id); //lock account
        reader_unlock(&bank_lock);
        if(from_index == -1) {
            printf("Source account not exists\n");
            return ERROR;
        }
        if(accounts[from_index].getPassword() != password){
            writer_unlock(&accounts[to_index].account_lock);
            writer_unlock(&accounts[from_index].account_lock);
            oss << "Error " << atm_id << ": Your transaction failed - password for account id "
            << from_account_id << " is incorrect";
            write_log(oss.str());
            return ERROR;   
        }
    }
    else{
        ostringstream oss; 
        from_index = getAccount_index_write(from_account_id); //lock account
        if(from_index == -1) {
            reader_unlock(&bank_lock);
            printf("Source account not exists\n");
            return ERROR;
        }
        if(accounts[from_index].getPassword() != password){
            writer_unlock(&accounts[from_index].account_lock);
            reader_unlock(&bank_lock);
            oss << "Error " << atm_id << ": Your transaction failed - password for account id "
            << from_account_id << " is incorrect";
            write_log(oss.str());
            return ERROR;   
        }
        to_index = getAccount_index_write(to_account_id); //lock account
        reader_unlock(&bank_lock);
        if(to_index == -1) {
            printf("Destination account not exists\n");
            return ERROR;
        }
    } 
    if(accounts[from_index].getBalance() < amount){
        if (from_account_id > to_account_id) { // swap if needed in order to avoid deadlock
            writer_unlock(&accounts[to_index].account_lock);
            writer_unlock(&accounts[from_index].account_lock);
        }
        else{ 
            writer_unlock(&accounts[from_index].account_lock);
            writer_unlock(&accounts[to_index].account_lock);
        }       
        oss << "Error " << atm_id << ": Your transaction failed - account id "
        << from_account_id << " is balance is lower than " << amount;
        write_log(oss.str());
        return ERROR;  
    }
    else{
        accounts[from_index].setBalance(accounts[from_index].getBalance()-amount);
        accounts[to_index].setBalance(accounts[to_index].getBalance()+amount);
        oss << atm_id << ": Transfer " << amount << " from account "
        << from_account_id << " to account " << to_account_id 
        << " new account balance is " << accounts[from_index].getBalance()
        << " new target account balance is " << accounts[to_index].getBalance(); //inside critical cuz using getBalance()

        if (from_account_id > to_account_id) { // swap if needed in order to avoid deadlock
            writer_unlock(&accounts[to_index].account_lock);
            writer_unlock(&accounts[from_index].account_lock);
        }
        else{ 
            writer_unlock(&accounts[from_index].account_lock);
            writer_unlock(&accounts[to_index].account_lock);
        } 
        write_log(oss.str());
        return SUCCESS; 
    }
return SUCCESS; 
}

int bank::close_atm(int target_atm_id, int killer_atm_id){
    if (cm) {printf("close atm\n");}

    ostringstream oss;
    int is_in = 0;
    //for (auto it = ATMs.begin(); it != ATMs.end(); ) {
    for (int i=0; i< static_cast<int>(ATMs.size()); i++) {
        if (ATMs[i]->id == target_atm_id) { // ATM is marked closed
            if (ATMs[i]->closed == true) {
                if (cm) {printf("in close_atm found atm that was already closed %d\n",target_atm_id);}
                oss << "Error " << killer_atm_id << ": Your close operation failed - ATM ID "
                << target_atm_id << " is already in a closed state";
                is_in = 2;
            }
            else {
                ATMs[i]->wanted_to_close = true;
                is_in = 1;
            }
        }
    }
    if (is_in == 0){
        if (cm) {printf("in close_atm atm dont exist\n");}

        oss << "Error " << killer_atm_id << ": Your transaction failed - ATM ID " << target_atm_id << " does not exist";
    }
    else if (is_in == 1){
        if (cm) {printf("in close_atm found atm to close %d\n",target_atm_id);}
        oss << "Bank: ATM " << killer_atm_id << " closed " << target_atm_id << " successfully";
    }
    write_log(oss.str());
    if (cm) {printf("finish atm close func \n");}

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

    if (cm) {printf("created bank\n");}

    main_bank->ATMs.reserve(atm_num); // for preventing accessing freed memory
    for (int i = 0; i < atm_num; i++) {
        if (cm) {printf("created ATM\n");}
    main_bank->ATMs.emplace_back(std::unique_ptr<ATM>(new ATM(i + 1, input_files[i], main_bank, false)));
        main_bank->ATMs[i]->start(); // call start on the constructed object
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
        // Check if 0.5s passed since last print
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_print_time).count() >= 500) {
            if (cm) {printf("in print\n");}
            last_print_time = now;
            main_bank->print_accounts();
        }

        // Check if 3s passed since last tax
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_tax_time).count() >= 3) {
            if (cm) {printf("in tax\n");}
            last_tax_time = now;
            main_bank->tax();
            main_bank->set_bank_balance(global_balance);
        }

        if (thread_counter == atm_num){
            for (int i = 0; i < atm_num; i++) {
                main_bank->ATMs[i]->join();
            }
            break;
        }
        // PERHAPS DELETE - tiny sleep to avoid CPU spinning at 100%
        usleep(1000);
    }
    if (cm) {printf("after while\n");}


    // 4) Clean up memory and close files
    delete main_bank;
    close_log();
    if (cm) {printf("ending...\n");}

    return 0;
}

