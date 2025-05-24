#include "includes.hpp"
#include "atm.hpp"
#include "bank.hpp"

using namespace std;
extern int cm;
extern int thread_counter;

/*=============================================================================
    Helper Functions
=============================================================================*/

/**
 * @brief Tokenize a line from the ATM input file into parts (command and args).
 * @param line: a full line from the ATM input file.
 * @return a vector of tokens (each word from the line).
 */
vector<string> parse(const string& line) {
    vector<string> tokens;
    istringstream iss(line);
    string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}



/*=============================================================================
    ATM Class Methods
=============================================================================*/

ATM::ATM(int id, FILE *input_file, bank *bank, bool closed) : 
    id(id), input_file(input_file), main_bank(bank), wanted_to_close(false), closed(false){};

ATM::~ATM() {
    if (input_file) {
        fclose(input_file);
        input_file = nullptr;
    }
}

int ATM::get_id() {return id;}                
//double ATM::get_balance() {return balance;}                 
//void ATM::set_balance(double amount) {balance = amount;}
pthread_t ATM::get_thread() {return atm_thread;}
void ATM::start() { 
    if(cm) printf("got into atm->start\n");
    pthread_create(&atm_thread, nullptr, runATM, this); 
    if(cm) printf("atm->start after pthread\n");
}
void ATM::join() {pthread_join(atm_thread, nullptr); }

int ATM::run()
{
    if (cm) {printf("ATM started\n");}
    char buffer[128]; // line buffer
    
    // if (!input_file) {
    //     printf("ERROR: input_file is NULL\n");
    // }
    while (fgets(buffer, sizeof(buffer), input_file) != nullptr)
    {
        if (cm) {printf("ATM read line: %s\n", buffer);}
        if (this->closed){ // ATM was requested to stop
            if (cm) {printf("ATM %d want to break, closed is %d\n", this->id, this->closed);}
            break;
        }        
        string line(buffer);
        vector<string> args = parse(line);
        execute(args); // 1 second delay inside.
        usleep(100000);   // Sleep for 100 miliSeconds
    }
    if (cm) {printf("1111111111111111111111111111111111111111111111111111111111111111111111\n");}
    thread_counter++; //global variable (Gil)
    if (cm) {printf("ATM finished, thread counter is: %d\n", thread_counter);}
    return SUCCESS;
}

int ATM::execute(const vector<string>& args) {
    if (cm) {printf("ATM execute\n");}
    if(args.empty()) return ERROR;
    sleep(1); 
    if (args[0] == "O")
        return main_bank->open_new_account(stoi(args[1]), stoi(args[2]), stod(args[3]), this->id);
    else if (args[0] == "D")
        return main_bank->deposit(stoi(args[1]), stoi(args[2]), stod(args[3]), this->id);
    else if (args[0] == "W")
        return main_bank->withdraw(stoi(args[1]), stoi(args[2]), stod(args[3]), this->id);
    else if (args[0] == "B")
        return main_bank->check_balance(stoi(args[1]), stoi(args[2]), this->id);
    else if (args[0] == "Q")
        return main_bank->close_account(stoi(args[1]), stoi(args[2]), this->id);
    else if (args[0] == "T")
        return main_bank->transfer(stoi(args[1]), stoi(args[2]), stoi(args[3]), stod(args[4]), this->id);   
    else if (args[0] == "C") 
        return main_bank->close_atm(stoi(args[1]), this->id);
    
    return ERROR; // Invalid command
}

void* ATM::runATM(void* arg) {
    ATM* atm = static_cast<ATM*>(arg);
    if (cm) {printf("ATM started wrapper func\n");}
    atm->run();
    return nullptr;
}
  



