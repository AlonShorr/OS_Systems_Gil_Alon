#ifndef ATM_H
#define ATM_H

#include "includes.hpp"
#include "bank.hpp"

class ATM {
    private:
        int id;                            
        double balance;
        FILE* input_file;                // File pointer for ATM input file
        bank* main_bank;                 // Pointer to the main bank object
        pthread_t atm_thread;

    public:
        /**
         * @brief: ATM constructor and destructor
         */
        ATM(int id, FILE* input_file, bank* bank);        
        ~ATM();

        /**
         * @brief: getters and setters for ATM fields
         */
        int get_id();                       
        double get_balance();                 
        void set_balance(double amount); 

        /**
         * @brief: funcions to manage ATM threads
         * @note: start() creates a new thread for the ATM
         *        join() waits for the ATM thread to finish (to later close the program)
         *        run() is the main function that executes the ATM logic  
         */
        void start();            
        void join();             
        int run();                     // Main ATM logic (command execution)

        /**
         * @brief Execute the ATM command based on the parsed arguments.
         * @param args: a vector of strings containing the command and its arguments.
         * @return: 0 on success, 1 on failure.
         */
        int ATM::execute(const vector<string> &args);

        /**
         * @brief runATM is a wrapper function for pthread library use
         * @note: calls ATM::run() to execute the ATM logic
         * @param arg: pointer to an ATM object
         */
        static void* runATM(void* arg); // wrapper for pthread library use
};

#endif // ATM_H