#include "BankingUI.h"

#include "Account.h"
#include "AccountManager.h"
#include "AuditLogger.h"
#include "InputHandling.h"

#include <cstdint>
#include <iostream>
#include <string>

namespace {

std::string idTypeName(int idType) {
    switch (idType) {
        case 1: return "SSN";
        case 2: return "ITIN";
        case 3: return "EIN";
        default: return "Unknown";
    }
}

void printMainMenu() {
    std::cout << "\n========================================\n"
              << "      GARY'S SECURE BANKING DEMO\n"
              << "========================================\n"
              << "1. Create a bank account\n"
              << "2. Log into an existing account\n"
              << "3. Exit\n"
              << "----------------------------------------\n";
}

void createAccountFlow(AccountManager& manager) {
    std::string username;
    while (true) {
        username = InputHandling::readLine("New username: ");

        if (!InputHandling::validateUsername(username)) {
            std::cout << "[SYSTEM] Username must be 3-24 characters and contain only "
                         "letters, numbers, '.', '_' or '-'.\n";
            continue;
        }

        if (manager.checkDuplicateUsernames(username)) {
            std::cout << "[SYSTEM] That username is already taken.\n";
            continue;
        }

        break;
    }

    std::string password;
    while (true) {
        password = InputHandling::readLine("New password (12-128 characters): ");

        if (!InputHandling::validatePassword(password)) {
            std::cout << "[SYSTEM] Password must be 12-128 characters and cannot contain control characters.\n";
            continue;
        }

        const std::string confirmation = InputHandling::readLine("Confirm password: ");
        if (password != confirmation) {
            std::cout << "[SYSTEM] Passwords did not match.\n";
            continue;
        }

        break;
    }

    const std::int64_t startingBalance = InputHandling::readMoneyCents(
        "Starting balance (example: 250.00): ", true
    );

    std::cout << "\nDemo identification type\n"
              << "1. SSN\n"
              << "2. ITIN\n"
              << "3. EIN\n";
    const int idType = InputHandling::readChoice("Choice: ", 1, 3);

    std::string idLastFour;
    while (true) {
        idLastFour = InputHandling::readLine(
            "Enter ONLY the last four digits of a fictional/demo identifier: "
        );

        if (InputHandling::validateIDLastFour(idLastFour)) {
            break;
        }

        std::cout << "[SYSTEM] Enter exactly four digits. Do not use a real SSN/ITIN/EIN in this demo.\n";
    }

    if (manager.createAccount(username, password, startingBalance, idType, idLastFour)) {
        std::cout << "[SYSTEM] Account created successfully.\n";
    } else {
        std::cout << "[SYSTEM] Account creation failed safely. No new account was saved.\n";
    }
}

void loggedInMenu(AccountManager& manager, Account& account, AuditLogger& logger) {
    while (true) {
        std::cout << "\n========================================\n"
                  << "Logged in as: " << account.getUsername() << '\n'
                  << "1. View balance\n"
                  << "2. Deposit\n"
                  << "3. Withdraw\n"
                  << "4. Change password\n"
                  << "5. View masked account profile\n"
                  << "6. Log out\n"
                  << "----------------------------------------\n";

        const int choice = InputHandling::readChoice("Choice: ", 1, 6);

        if (choice == 1) {
            std::cout << "Current balance: "
                      << InputHandling::formatMoney(account.getBalanceCents()) << '\n';
        }
        else if (choice == 2) {
            const std::int64_t amount = InputHandling::readMoneyCents("Deposit amount: ");
            if (account.deposit(amount)) {
                if (manager.saveToDatabaseFile()) {
                    logger.log("DEPOSIT", account.getUsername());
                    std::cout << "[SYSTEM] Deposit completed. New balance: "
                              << InputHandling::formatMoney(account.getBalanceCents()) << '\n';
                } else {
                    account.withdraw(amount); // rollback the in-memory change
                    std::cout << "[SYSTEM] Deposit was rolled back because it could not be saved.\n";
                }
            }
        }
        else if (choice == 3) {
            const std::int64_t amount = InputHandling::readMoneyCents("Withdrawal amount: ");
            if (!account.withdraw(amount)) {
                std::cout << "[SYSTEM] Withdrawal rejected. Check the amount and available balance.\n";
                continue;
            }

            if (manager.saveToDatabaseFile()) {
                logger.log("WITHDRAWAL", account.getUsername());
                std::cout << "[SYSTEM] Withdrawal completed. New balance: "
                          << InputHandling::formatMoney(account.getBalanceCents()) << '\n';
            } else {
                account.deposit(amount); // rollback the in-memory change
                std::cout << "[SYSTEM] Withdrawal was rolled back because it could not be saved.\n";
            }
        }
        else if (choice == 4) {
            const std::string currentPassword = InputHandling::readLine("Current password: ");
            if (!account.authenticate(currentPassword)) {
                logger.log("PASSWORD_CHANGE_REJECTED", account.getUsername());
                std::cout << "[SYSTEM] Current password was incorrect.\n";
                continue;
            }

            std::string newPassword;
            while (true) {
                newPassword = InputHandling::readLine("New password (12-128 characters): ");
                if (!InputHandling::validatePassword(newPassword)) {
                    std::cout << "[SYSTEM] New password does not meet the length/input rules.\n";
                    continue;
                }

                const std::string confirmation = InputHandling::readLine("Confirm new password: ");
                if (newPassword != confirmation) {
                    std::cout << "[SYSTEM] Passwords did not match.\n";
                    continue;
                }
                break;
            }

            const Account oldAccount = account;
            account.changePassword(newPassword);

            if (manager.saveToDatabaseFile()) {
                logger.log("PASSWORD_CHANGED", account.getUsername());
                std::cout << "[SYSTEM] Password updated with a new salt and current work factor.\n";
            } else {
                account = oldAccount;
                std::cout << "[SYSTEM] Password change was rolled back because it could not be saved.\n";
            }
        }
        else if (choice == 5) {
            std::cout << "Username: " << account.getUsername() << '\n'
                      << "Identifier type: " << idTypeName(account.getPrimaryIDType()) << '\n'
                      << "Stored identifier: ***-**-" << account.getIDLastFour() << '\n'
                      << "Password storage: PBKDF2-HMAC-SHA256, unique salt, "
                      << account.getPasswordIterations() << " iterations\n";
        }
        else if (choice == 6) {
            logger.log("LOGOUT", account.getUsername());
            std::cout << "[SYSTEM] Logged out.\n";
            return;
        }
    }
}

void loginFlow(AccountManager& manager, AuditLogger& logger) {
    const std::string username = InputHandling::readLine("Username: ");
    const std::string password = InputHandling::readLine("Password: ");

    std::string statusMessage;
    Account* account = manager.authenticateUser(username, password, statusMessage);
    std::cout << "[SYSTEM] " << statusMessage << '\n';

    if (account != nullptr) {
        loggedInMenu(manager, *account, logger);
    }
}

} // namespace

namespace BankingUI {

void run(AccountManager& manager, AuditLogger& logger) {
    std::cout << "Created by Gary Pleasant\n"
              << "Educational secure-coding project - NOT a real banking system.\n"
              << "Loaded accounts: " << manager.accountCount() << "\n";

    while (true) {
        printMainMenu();
        const int choice = InputHandling::readChoice("Choice: ", 1, 3);

        if (choice == 1) {
            createAccountFlow(manager);
        } else if (choice == 2) {
            loginFlow(manager, logger);
        } else {
            logger.log("PROGRAM_EXIT");
            std::cout << "Goodbye.\n";
            break;
        }
    }
}

} // namespace BankingUI
