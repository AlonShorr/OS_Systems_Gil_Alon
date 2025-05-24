#ifndef ATM_H
#define ATM_H
#include "includes.hpp"
using namespace std;

class bank;

class ATM {
    public:
        int id;                            
        //double balance;
        FILE* input_file;                // File pointer for ATM input file
        bank* main_bank;                 // Pointer to the main bank object
        bool wanted_to_close;            // Flag to indicate if someone wants to close this ATM
        bool closed;                     // Flag to indicate if the ATM is closed
        pthread_t atm_thread;

        /**
         * @brief: ATM constructor and destructor
         */
        ATM(int id, FILE* input_file, bank* bank, bool closed);
               
        ~ATM();

        /**
         * @brief: getters and setters for ATM fields
         */
        int get_id();                       
        //double get_balance();                 
        //void set_balance(double amount);
        pthread_t get_thread();

        /**
         * @brief: funcions to manage ATM threads
         * @note: start() creates a new thread for the ATM
         *        join() waits for the ATM thread to finish (to later close the program)
         *        run() is the main function that executes the ATM logic  
         */
        void start();            
        void join();             
        int run();   // Main ATM logic (command execution)

        /**
         * @brief Execute the ATM command based on the parsed arguments.
         * @param args: a vector of strings containing the command and its arguments.
         * @return: 0 on success, 1 on failure.
         */
        int execute(const vector<string> &args);

        /**
         * @brief runATM is a wrapper function for pthread library use
         * @note: calls ATM::run() to execute the ATM logic
         * @param arg: pointer to an ATM object
         */
        static void* runATM(void* arg); // wrapper for pthread library use
};

#endif // ATM_H