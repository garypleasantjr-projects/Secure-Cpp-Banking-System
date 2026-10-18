#include "Account.h"
#include "AccountManager.h"
#include "AuditLogger.h"
#include "Crypto.h"
#include "InputHandling.h"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <system_error>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << '\n';
        std::exit(1);
    }
}

}

int main() {
    // Input validation tests
    require(InputHandling::validateUsername("gary.dev"), "valid username should pass");
    require(!InputHandling::validateUsername("bad username"), "username with spaces should fail");
    require(
        InputHandling::validatePassword("correct-horse-battery"),
        "valid password should pass"
    );
    require(!InputHandling::validatePassword("short"), "short password should fail");
    require(InputHandling::validateIDLastFour("1234"), "four ID digits should pass");
    require(!InputHandling::validateIDLastFour("12A4"), "non-digit ID input should fail");

    std::int64_t cents = 0;
    require(InputHandling::parseMoneyToCents("125.50", cents), "money parser should accept 125.50");
    require(cents == 12550, "125.50 should become 12550 cents");
    require(InputHandling::parseMoneyToCents("10", cents), "money parser should accept whole dollars");
    require(cents == 1000, "10 dollars should become 1000 cents");
    require(!InputHandling::parseMoneyToCents("10.999", cents), "more than two decimals should fail");

    // Cryptography tests
    const auto salt = Crypto::generateSalt();
    const auto hash1 = Crypto::generatePasswordHash("correct-horse-battery", salt);
    const auto hash2 = Crypto::generatePasswordHash("correct-horse-battery", salt);
    const auto wrong = Crypto::generatePasswordHash("wrong-password-value", salt);

    require(Crypto::constantTimeEqual(hash1, hash2), "same password and salt should match");
    require(!Crypto::constantTimeEqual(hash1, wrong), "different password should not match");

    const std::string hex = Crypto::bytesToHex(hash1);
    require(
        Crypto::constantTimeEqual(hash1, Crypto::hexToBytes(hex)),
        "hex encoding should round-trip"
    );

    // Account behavior tests
    Account account("gary", "correct-horse-battery", 10000, 1, "1234");
    require(account.authenticate("correct-horse-battery"), "correct password should authenticate");
    require(!account.authenticate("wrong-password-value"), "wrong password should fail");
    require(account.getBalanceCents() == 10000, "starting balance should be preserved");

    require(account.deposit(2500), "normal deposit should succeed");
    require(account.getBalanceCents() == 12500, "deposit should update balance");
    require(account.withdraw(3000), "normal withdrawal should succeed");
    require(account.getBalanceCents() == 9500, "withdrawal should update balance");
    require(!account.withdraw(20000), "overdraft should fail");
    require(account.getBalanceCents() == 9500, "failed withdrawal should not change balance");

    const std::int64_t nearMaximum = std::numeric_limits<std::int64_t>::max() - 5;
    Account overflowAccount("limit.user", "correct-horse-battery", nearMaximum, 1, "5678");
    require(!overflowAccount.deposit(10), "overflowing deposit should fail");
    require(
        overflowAccount.getBalanceCents() == nearMaximum,
        "failed overflow deposit should preserve balance"
    );

    account.changePassword("new-correct-horse-battery");
    require(!account.authenticate("correct-horse-battery"), "old password should stop working");
    require(account.authenticate("new-correct-horse-battery"), "new password should authenticate");

    // Persistence, duplicate-user, and lockout tests
    namespace fs = std::filesystem;
    const fs::path testDirectory = fs::temp_directory_path() / "gary_secure_bank_tests";
    const fs::path databasePath = testDirectory / "accounts.db";
    const fs::path auditPath = testDirectory / "audit.log";
    const fs::path malformedPath = testDirectory / "malformed.db";

    std::error_code cleanupError;
    fs::remove_all(testDirectory, cleanupError);
    fs::create_directories(testDirectory);

    {
        AuditLogger logger(auditPath.string());
        AccountManager manager(databasePath.string(), logger);

        require(manager.loadDatabaseFile(), "missing database should be treated as first launch");
        require(
            manager.createAccount(
                "persisted.user",
                "correct-horse-battery",
                5000,
                1,
                "2468"
            ),
            "account creation should persist successfully"
        );
        require(
            !manager.createAccount(
                "persisted.user",
                "another-valid-password",
                5000,
                1,
                "2468"
            ),
            "duplicate username should be rejected"
        );
    }

    {
        AuditLogger logger(auditPath.string());
        AccountManager manager(databasePath.string(), logger);

        require(manager.loadDatabaseFile(), "saved database should reload");
        require(manager.accountCount() == 1, "reloaded database should contain one account");

        std::string statusMessage;
        Account* loaded = manager.authenticateUser(
            "persisted.user",
            "correct-horse-battery",
            statusMessage
        );
        require(loaded != nullptr, "persisted password should authenticate after reload");
        require(loaded->getBalanceCents() == 5000, "persisted balance should survive reload");

        for (int attempt = 0; attempt < AccountManager::MAX_FAILED_ATTEMPTS; ++attempt) {
            require(
                manager.authenticateUser(
                    "persisted.user",
                    "wrong-password-value",
                    statusMessage
                ) == nullptr,
                "wrong password should be rejected"
            );
        }

        require(
            statusMessage.find("temporarily locked") != std::string::npos,
            "fifth failed attempt should trigger temporary lockout"
        );
        require(
            manager.authenticateUser(
                "persisted.user",
                "correct-horse-battery",
                statusMessage
            ) == nullptr,
            "correct password should remain blocked during lockout"
        );
    }

    {
        const std::string saltHex(Crypto::SALT_LENGTH * 2, 'a');
        const std::string hashHex(Crypto::KEY_LENGTH * 2, 'b');

        std::ofstream malformed(malformedPath);
        malformed << "SECURE_BANK_DB_V1\n"
                  << "broken.user|" << saltHex << '|' << hashHex
                  << "|600000junk|5000|1|1234\n";
        malformed.close();

        AuditLogger logger(auditPath.string());
        AccountManager manager(malformedPath.string(), logger);
        require(
            !manager.loadDatabaseFile(),
            "database numeric fields with trailing junk should be rejected"
        );
    }

    fs::remove_all(testDirectory, cleanupError);

    std::cout << "All core and manager tests passed.\n";
    return 0;
}
