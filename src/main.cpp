#include "AccountManager.h"
#include "AuditLogger.h"
#include "BankingUI.h"

#include <exception>
#include <filesystem>
#include <iostream>

// MAIN - STARTUP AND ERROR HANDLING
int main() {
    try {
        // create the storage directory before opening its database and log files.
        std::filesystem::create_directories("data");

        ///// INITIALIZE LOGGING AND ACCOUNT STORAGE /////
        AuditLogger logger("data/audit.log");
        AccountManager manager("data/accounts.db", logger);

        if (!manager.loadDatabaseFile()) {
            std::cerr << "[FATAL] Existing database could not be loaded safely.\n"
                      << "The program will exit rather than overwrite questionable data.\n";
            return 1; // exit on an unrecoverable startup error
        }

        // the UI handles menus; main handles startup and fatal errors.
        BankingUI::run(manager, logger);
    } catch (const std::exception& error) {
        std::cerr << "[FATAL] " << error.what() << '\n';
        return 1;
    }

    return 0;
}
