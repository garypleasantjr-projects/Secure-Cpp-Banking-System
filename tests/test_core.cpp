#include "Account.h"
#include "Crypto.h"
#include "InputHandling.h"

#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    // Input validation tests
    assert(InputHandling::validateUsername("gary.dev"));
    assert(!InputHandling::validateUsername("bad username"));
    assert(InputHandling::validatePassword("correct-horse-battery"));
    assert(!InputHandling::validatePassword("short"));
    assert(InputHandling::validateIDLastFour("1234"));
    assert(!InputHandling::validateIDLastFour("12A4"));

    std::int64_t cents = 0;
    assert(InputHandling::parseMoneyToCents("125.50", cents));
    assert(cents == 12550);
    assert(InputHandling::parseMoneyToCents("10", cents));
    assert(cents == 1000);
    assert(!InputHandling::parseMoneyToCents("10.999", cents));

    // Cryptography tests
    const auto salt = Crypto::generateSalt();
    const auto hash1 = Crypto::generatePasswordHash("correct-horse-battery", salt);
    const auto hash2 = Crypto::generatePasswordHash("correct-horse-battery", salt);
    const auto wrong = Crypto::generatePasswordHash("wrong-password-value", salt);

    assert(Crypto::constantTimeEqual(hash1, hash2));
    assert(!Crypto::constantTimeEqual(hash1, wrong));

    const std::string hex = Crypto::bytesToHex(hash1);
    assert(Crypto::constantTimeEqual(hash1, Crypto::hexToBytes(hex)));

    // Account behavior tests
    Account account("gary", "correct-horse-battery", 10000, 1, "1234");
    assert(account.authenticate("correct-horse-battery"));
    assert(!account.authenticate("wrong-password-value"));
    assert(account.getBalanceCents() == 10000);

    assert(account.deposit(2500));
    assert(account.getBalanceCents() == 12500);
    assert(account.withdraw(3000));
    assert(account.getBalanceCents() == 9500);
    assert(!account.withdraw(20000));
    assert(account.getBalanceCents() == 9500);

    account.changePassword("new-correct-horse-battery");
    assert(!account.authenticate("correct-horse-battery"));
    assert(account.authenticate("new-correct-horse-battery"));

    std::cout << "All core tests passed.\n";
    return 0;
}
