#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <vector>

// CRYPTO CONSTANTS AND FUNCTIONS
namespace Crypto {

// PBKDF2 work factor for newly created or changed passwords.
///// CURRENT PASSWORD-HASH SETTINGS /////
constexpr int CURRENT_ITERATIONS = 600000;
constexpr int KEY_LENGTH = 32; // 32-byte derived password key
// generate 16 random bytes; keep saved salts for older accounts.
constexpr int SALT_LENGTH = 16;

std::vector<unsigned char> generateSalt();
std::vector<unsigned char> generatePasswordHash(
    const std::string& password,
    const std::vector<unsigned char>& salt,
    int iterations = CURRENT_ITERATIONS
);

bool constantTimeEqual(
    const std::vector<unsigned char>& left,
    const std::vector<unsigned char>& right
);

// hex is a text representation of bytes, not encryption.
std::string bytesToHex(const std::vector<unsigned char>& bytes);
std::vector<unsigned char> hexToBytes(const std::string& hex);

} // namespace Crypto

#endif
