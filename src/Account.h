#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <cstdint>
#include <string>
#include <vector>

// An Account owns its balance and password-verification information.
class Account {
private:
    std::string username;
    // Raw salt bytes, not text; each new password gets a random salt.
    std::vector<unsigned char> salt;
    std::vector<unsigned char> passwordHash;
    int passwordIterations;
    // Store money as integer cents rather than floating-point dollars.
    std::int64_t balanceCents;
    int primaryIDType;
    std::string idLastFour;

public:
    // Constructor used when creating a brand-new account from a plaintext password.
    Account(
        const std::string& username,
        const std::string& password,
        std::int64_t startingBalanceCents,
        int primaryIDType,
        const std::string& idLastFour
    );

    // Constructor used only when loading an already-hashed account from disk.
    Account(
        const std::string& username,
        const std::vector<unsigned char>& salt,
        const std::vector<unsigned char>& passwordHash,
        int passwordIterations,
        std::int64_t balanceCents,
        int primaryIDType,
        const std::string& idLastFour
    );

    const std::string& getUsername() const;
    const std::vector<unsigned char>& getSalt() const;
    const std::vector<unsigned char>& getPasswordHash() const;
    int getPasswordIterations() const;
    std::int64_t getBalanceCents() const;
    int getPrimaryIDType() const;
    const std::string& getIDLastFour() const;

    bool authenticate(const std::string& attemptedPassword) const;
    void changePassword(const std::string& newPassword);

    bool deposit(std::int64_t amountCents);
    bool withdraw(std::int64_t amountCents);
};

#endif
