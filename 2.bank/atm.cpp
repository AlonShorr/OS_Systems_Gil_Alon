#include "includes.hpp"
#include "atm.hpp"

using namespace std;

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

ATM::ATM(int id, FILE *input_file, bank *bank, bool closed = false) : 
    id(id), input_file(input_file), main_bank(bank), balance(0){};

ATM::~ATM() {
    if (input_file) {
        fclose(input_file);
        input_file = nullptr;
    }
}

int ATM::get_id() {return id;}                
double ATM::get_balance() {return balance;}                 
void ATM::set_balance(double amount) {balance = amount;}
pthread_t ATM::get_thread() {return atm_thread;}
void ATM::start() { pthread_create(&atm_thread, nullptr, runATM, this); }
void ATM::join() { pthread_join(atm_thread, nullptr); }

int ATM::run()
{
    char buffer[128]; // line buffer
    int res = 0;
    while (fgets(buffer, sizeof(buffer), input_file) != nullptr)
    {
        if (this->closed) // ATM was requested to stop
            break;        
        string line(buffer);
        vector<string> args = parse(line);
        res = execute(args); // 1 second delay inside.
        if (res == ERROR)
            return ERROR; // Invalid command
        usleep(100000);   // Sleep for 100 miliSeconds
    }
    thread_counter++; //global variable (Gil)
    return SUCCESS;
}

int ATM::execute(const vector<string>& args) {
    if(args.empty()) return;
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
        return main_bank->transfer(stoi(args[1]), stoi(args[2]), stod(args[3]), this->id);
    else if (args[0] == "C") 
        return main_bank->close_atm(stoi(args[1]), this->id);
    
    return ERROR; // Invalid command
}

void* ATM::runATM(void* arg) {
    ATM* atm = static_cast<ATM*>(arg);
    atm->run();
    return nullptr;
}
  



