/*#
include "includes.hpp"
#include "atm.hpp"
#include "bank.hpp"
#include "logger.hpp" // TODO: think about include path

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
*/
