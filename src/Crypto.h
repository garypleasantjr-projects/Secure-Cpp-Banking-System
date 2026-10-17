#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <vector>

namespace Crypto {

constexpr int CURRENT_ITERATIONS = 600000;
constexpr int KEY_LENGTH = 32;
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

std::string bytesToHex(const std::vector<unsigned char>& bytes);
std::vector<unsigned char> hexToBytes(const std::string& hex);

} // namespace Crypto

#endif
