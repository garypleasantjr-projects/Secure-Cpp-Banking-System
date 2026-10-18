#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include "Account.h"
#include "AuditLogger.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class AccountManager {
private:
    struct LoginAttemptState {
        int failedAttempts = 0;
        std::chrono::steady_clock::time_point lockedUntil{};
    };

    // This vector owns all Account objects in memory
    std::vector<Account> accounts;
    std::string databaseFileName;
    // Reference: use main's logger rather than creating a copy
    AuditLogger& logger;
    // Failed-attempt counters and lockout times are stored only in memory
    std::unordered_map<std::string, LoginAttemptState> loginAttempts;

    // Non-owning pointer; nullptr means the username was not found
    Account* findAccount(const std::string& username);
    const Account* findAccount(const std::string& username) const;

public:
    static constexpr int MAX_FAILED_ATTEMPTS = 5;
    static constexpr int LOCKOUT_SECONDS = 30;

    AccountManager(const std::string& databaseFileName, AuditLogger& logger);

    bool createAccount(
        const std::string& username,
        const std::string& password,
        std::int64_t startingBalanceCents,
        int primaryIDType,
        const std::string& idLastFour
    );

    Account* authenticateUser(
        const std::string& username,
        const std::string& password,
        std::string& statusMessage
    );

    bool checkDuplicateUsernames(const std::string& username) const;
    bool loadDatabaseFile();
    bool saveToDatabaseFile() const;

    const std::string& whichDatabaseFile() const;
    void changeDatabaseFile(const std::string& newDatabaseFileName);
    std::size_t accountCount() const;
};

#endif
