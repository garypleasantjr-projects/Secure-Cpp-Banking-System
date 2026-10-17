#include "Account.h"
#include "Crypto.h"

#include <stdexcept>
#include <limits>


// NEW ACCOUNT CONSTRUCTOR
Account::Account(
    const std::string& username,
    const std::string& password,
    std::int64_t startingBalanceCents,
    int primaryIDType,
    const std::string& idLastFour
)
    : username(username),
      // new accounts receive a fresh random salt.
      salt(Crypto::generateSalt()),
      passwordHash(Crypto::generatePasswordHash(password, salt)),
      passwordIterations(Crypto::CURRENT_ITERATIONS),
      balanceCents(startingBalanceCents),
      primaryIDType(primaryIDType),
      idLastFour(idLastFour) {

    if (startingBalanceCents < 0) {
        throw std::invalid_argument("Starting balance cannot be negative.");
    }
}

// LOAD-SAVED-ACCOUNT CONSTRUCTOR
Account::Account(
    const std::string& username,
    const std::vector<unsigned char>& salt,
    const std::vector<unsigned char>& passwordHash,
    int passwordIterations,
    std::int64_t balanceCents,
    int primaryIDType,
    const std::string& idLastFour
)
    : username(username),
      // reloaded accounts must keep their saved salt to verify existing passwords.
      salt(salt),
      passwordHash(passwordHash),
      passwordIterations(passwordIterations),
      balanceCents(balanceCents),
      primaryIDType(primaryIDType),
      idLastFour(idLastFour) {

    if (salt.empty() || passwordHash.empty()) {
        throw std::invalid_argument("Loaded account is missing password security data.");
    }

    if (passwordIterations <= 0 || balanceCents < 0) {
        throw std::invalid_argument("Loaded account contains invalid numeric values.");
    }
}

///// GETTER IMPLEMENTATIONS /////
const std::string& Account::getUsername() const {
    return username;
}

const std::vector<unsigned char>& Account::getSalt() const {
    return salt;
}

const std::vector<unsigned char>& Account::getPasswordHash() const {
    return passwordHash;
}

int Account::getPasswordIterations() const {
    return passwordIterations;
}

std::int64_t Account::getBalanceCents() const {
    return balanceCents;
}

int Account::getPrimaryIDType() const {
    return primaryIDType;
}

const std::string& Account::getIDLastFour() const {
    return idLastFour;
}

// PASSWORD AUTHENTICATION LOGIC
bool Account::authenticate(const std::string& attemptedPassword) const {
    const std::vector<unsigned char> attemptedHash =
        Crypto::generatePasswordHash(attemptedPassword, salt, passwordIterations);

    return Crypto::constantTimeEqual(attemptedHash, passwordHash); // compare derived and stored hash bytes
}

// CHANGE PASSWORD WITH A NEW SALT
void Account::changePassword(const std::string& newPassword) {
    // password changes receive a fresh random salt and the current work factor.
    salt = Crypto::generateSalt();
    passwordIterations = Crypto::CURRENT_ITERATIONS;
    passwordHash = Crypto::generatePasswordHash(newPassword, salt, passwordIterations);
}

// ACCOUNT OPERATIONS - DEPOSIT
bool Account::deposit(std::int64_t amountCents) {
    // reject non-positive deposits and prevent signed integer overflow.
    if (amountCents <= 0 ||
        amountCents > std::numeric_limits<std::int64_t>::max() - balanceCents) {
        return false;
    }

    // change the balance only after the validation succeeds.
    balanceCents += amountCents;
    return true;
}

// ACCOUNT OPERATIONS - WITHDRAW
bool Account::withdraw(std::int64_t amountCents) {
    if (amountCents <= 0 || amountCents > balanceCents) {
        return false;
    }

    balanceCents -= amountCents; // subtract cents after checking available funds
    return true;
}
