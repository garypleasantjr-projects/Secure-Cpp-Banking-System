#include "Crypto.h"

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace Crypto {

std::vector<unsigned char> generateSalt() {
    std::vector<unsigned char> salt(SALT_LENGTH);

    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("Cryptographically secure salt generation failed.");
    }

    return salt;
}

std::vector<unsigned char> generatePasswordHash(
    const std::string& password,
    const std::vector<unsigned char>& salt,
    int iterations
) {
    if (salt.empty()) {
        throw std::invalid_argument("Salt cannot be empty.");
    }

    if (iterations <= 0) {
        throw std::invalid_argument("PBKDF2 iteration count must be positive.");
    }

    std::vector<unsigned char> hash(KEY_LENGTH);

    const int success = PKCS5_PBKDF2_HMAC(
        password.data(),
        static_cast<int>(password.size()),
        salt.data(),
        static_cast<int>(salt.size()),
        iterations,
        EVP_sha256(),
        static_cast<int>(hash.size()),
        hash.data()
    );

    if (success != 1) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("PBKDF2 password hashing failed.");
    }

    return hash;
}

bool constantTimeEqual(
    const std::vector<unsigned char>& left,
    const std::vector<unsigned char>& right
) {
    if (left.size() != right.size() || left.empty()) {
        return false;
    }

    return CRYPTO_memcmp(left.data(), right.data(), left.size()) == 0;
}

std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    std::ostringstream output;
    output << std::hex << std::setfill('0');

    for (unsigned char byte : bytes) {
        output << std::setw(2) << static_cast<int>(byte);
    }

    return output.str();
}

std::vector<unsigned char> hexToBytes(const std::string& hex) {
    if (hex.size() % 2 != 0) {
        throw std::invalid_argument("Hex input must contain an even number of characters.");
    }

    std::vector<unsigned char> bytes;
    bytes.reserve(hex.size() / 2);

    for (std::size_t i = 0; i < hex.size(); i += 2) {
        const std::string byteText = hex.substr(i, 2);
        std::size_t parsedCharacters = 0;
        const unsigned long value = std::stoul(byteText, &parsedCharacters, 16);

        if (parsedCharacters != 2 || value > 255) {
            throw std::invalid_argument("Invalid hexadecimal data.");
        }

        bytes.push_back(static_cast<unsigned char>(value));
    }

    return bytes;
}

} // namespace Crypto
