#include "AccountManager.h"
#include "Crypto.h"
#include "InputHandling.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <system_error>

// FILE-LOCAL DATABASE HELPERS
namespace {
// version marker: older or corrupted formats cannot silently be treated as valid.
constexpr const char* DATABASE_HEADER = "SECURE_BANK_DB_V1";

// divide the saved line into its pipe-separated fields.
std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream stream(line);
    std::string part;

    while (std::getline(stream, part, delimiter)) {
        parts.push_back(part);
    }

    return parts;
}
}

AccountManager::AccountManager(const std::string& databaseFileName, AuditLogger& logger)
    : databaseFileName(databaseFileName), logger(logger) {}

Account* AccountManager::findAccount(const std::string& username) {
    for (Account& account : accounts) {
        if (account.getUsername() == username) {
            return &account; // return the address of the matching Account
        }
    }
    return nullptr;
}

const Account* AccountManager::findAccount(const std::string& username) const {
    for (const Account& account : accounts) {
        if (account.getUsername() == username) {
            return &account;
        }
    }
    return nullptr;
}

bool AccountManager::checkDuplicateUsernames(const std::string& username) const {
    return findAccount(username) != nullptr;
}

///// ACCOUNT CREATION AND INITIAL SAVE /////
bool AccountManager::createAccount(
    const std::string& username,
    const std::string& password,
    std::int64_t startingBalanceCents,
    int primaryIDType,
    const std::string& idLastFour
) {
    if (checkDuplicateUsernames(username)) {
        return false;
    }

    try {
        // construct a new Account directly inside the manager's vector.
        accounts.emplace_back(
            username,
            password,
            startingBalanceCents,
            primaryIDType,
            idLastFour
        );

        // on a normal save failure, undo the just-created in-memory account.
        if (!saveToDatabaseFile()) {
            accounts.pop_back(); // undo this account addition in memory
            return false;
        }

        logger.log("ACCOUNT_CREATED", username);
        return true;
    } catch (const std::exception& error) {
        std::cerr << "[ERROR] Account creation failed: " << error.what() << '\n';
        return false;
    }
}

// USER AUTHENTICATION AND LOGIN LOCKOUT
Account* AccountManager::authenticateUser(
    const std::string& username,
    const std::string& password,
    std::string& statusMessage
) {
    // get or create the login-attempt record for the requested username.
    LoginAttemptState& state = loginAttempts[username];
    const auto now = std::chrono::steady_clock::now();

    if (state.lockedUntil > now) {
        const auto secondsRemaining = std::chrono::duration_cast<std::chrono::seconds>(
            state.lockedUntil - now
        ).count() + 1;

        statusMessage = "Too many failed attempts. Try again in "
            + std::to_string(secondsRemaining) + " seconds.";
        logger.log("LOGIN_BLOCKED", username);
        return nullptr;
    }

    Account* account = findAccount(username);
    bool authenticated = false;

    try {
        if (account != nullptr) {
            authenticated = account->authenticate(password);
        } else {
            // do roughly the same expensive password-hash work even when the username
            // does not exist. This reduces the obvious timing difference between
            // "known user" and "unknown user" login attempts.
            const std::vector<unsigned char> dummySalt(Crypto::SALT_LENGTH, 0xA5);
            (void)Crypto::generatePasswordHash(password, dummySalt);
        }
    } catch (const std::exception&) {
        statusMessage = "Authentication could not be completed safely.";
        logger.log("LOGIN_ERROR", username);
        return nullptr;
    }

    if (authenticated) {
        state.failedAttempts = 0;
        state.lockedUntil = std::chrono::steady_clock::time_point{};
        statusMessage = "Login successful.";
        logger.log("LOGIN_SUCCESS", username);
        return account;
    }

    state.failedAttempts++;
    logger.log("LOGIN_FAILURE", username);

    if (state.failedAttempts >= MAX_FAILED_ATTEMPTS) {
        state.failedAttempts = 0;
        state.lockedUntil = now + std::chrono::seconds(LOCKOUT_SECONDS);
        statusMessage = "Login failed. Too many attempts; this username is temporarily locked.";
    } else {
        statusMessage = "Login failed. Username or password is incorrect.";
    }

    return nullptr;
}

// DATABASE LOADING - VALIDATE BEFORE COMMIT
bool AccountManager::loadDatabaseFile() {
    std::ifstream input(databaseFileName);

    if (!input) {
        // A missing database on first launch is normal. Start empty.
        return true;
    }

    std::string header;
    if (!std::getline(input, header) || header != DATABASE_HEADER) {
        std::cerr << "[ERROR] Database header is missing or unsupported.\n";
        return false;
    }

    // load into a temporary collection before replacing the existing accounts.
    std::vector<Account> loadedAccounts;
    std::string line;
    std::size_t lineNumber = 1;

    try {
        while (std::getline(input, line)) {
            lineNumber++;
            if (line.empty()) {
                continue;
            }

            const std::vector<std::string> fields = split(line, '|');
            if (fields.size() != 7) { // each record must contain exactly seven fields
                throw std::runtime_error("Expected 7 fields on line " + std::to_string(lineNumber));
            }

            const std::string& username = fields[0];
            const std::vector<unsigned char> salt = Crypto::hexToBytes(fields[1]);
            const std::vector<unsigned char> hash = Crypto::hexToBytes(fields[2]);
            const int iterations = std::stoi(fields[3]);
            const std::int64_t balanceCents = std::stoll(fields[4]);
            const int idType = std::stoi(fields[5]);
            const std::string& idLastFour = fields[6];

            if (!InputHandling::validateUsername(username)
                || salt.size() != Crypto::SALT_LENGTH
                || hash.size() != Crypto::KEY_LENGTH
                || iterations <= 0
                || balanceCents < 0
                || !InputHandling::validateIDType(idType)
                || !InputHandling::validateIDLastFour(idLastFour)) {
                throw std::runtime_error("Validation failed on line " + std::to_string(lineNumber));
            }

            const bool duplicate = std::any_of(
                loadedAccounts.begin(), loadedAccounts.end(),
                [&username](const Account& account) {
                    return account.getUsername() == username;
                }
            );

            if (duplicate) {
                throw std::runtime_error("Duplicate username on line " + std::to_string(lineNumber));
            }

            loadedAccounts.emplace_back(
                username,
                salt,
                hash,
                iterations,
                balanceCents,
                idType,
                idLastFour
            );
        }
    } catch (const std::exception& error) {
        std::cerr << "[ERROR] Database load failed safely: " << error.what() << '\n';
        return false;
    }

    // commit the loaded accounts only after validation succeeds.
    accounts = std::move(loadedAccounts);
    logger.log("DATABASE_LOADED"); // record the successful database load
    return true;
}

///// DATABASE SAVE - WRITE TEMP FILE BEFORE REPLACING /////
bool AccountManager::saveToDatabaseFile() const {
    // prepare a complete replacement before touching the real database.
    const std::string tempFile = databaseFileName + ".tmp";

    try {
        std::ofstream output(tempFile);

        if (!output) {
            std::cerr << "[ERROR] Could not open temporary database file.\n";
            return false;
        }

        output << DATABASE_HEADER << '\n';

        for (const Account& account : accounts) {
            output << account.getUsername() << '|'
                   << Crypto::bytesToHex(account.getSalt()) << '|'
                   << Crypto::bytesToHex(account.getPasswordHash()) << '|'
                   << account.getPasswordIterations() << '|'
                   << account.getBalanceCents() << '|'
                   << account.getPrimaryIDType() << '|'
                   << account.getIDLastFour() << '\n';
        }

        // check for file-writing errors after closing the temporary file.
        output.close();

        if (!output) {
            std::error_code ignored;
            std::filesystem::remove(tempFile, ignored);

            std::cerr << "[ERROR] Database write failed; old database kept.\n";
            return false;
        }

        // replace the old file only after the new one has been written.
        std::filesystem::rename(tempFile, databaseFileName);

        return true;

    } catch (const std::exception& error) {
        std::error_code ignored;
        std::filesystem::remove(tempFile, ignored);

        std::cerr << "[ERROR] Database save failed: " << error.what() << '\n';
        return false;
    }
}


const std::string& AccountManager::whichDatabaseFile() const {
    return databaseFileName;
}

void AccountManager::changeDatabaseFile(const std::string& newDatabaseFileName) {
    databaseFileName = newDatabaseFileName;
}

std::size_t AccountManager::accountCount() const {
    return accounts.size();
}
