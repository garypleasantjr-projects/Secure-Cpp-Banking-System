#include "InputHandling.h"

#include <cctype>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

// INPUT HANDLING IMPLEMENTATIONS
namespace InputHandling {

// READ A FULL CONSOLE LINE
std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;

    // read the entire console line and report EOF or another input-stream failure.
    if (!std::getline(std::cin, value)) {
        throw std::runtime_error("Input stream closed unexpectedly.");
    }

    return value;
}

// MENU CHOICE VALIDATION
int readChoice(const std::string& prompt, int min, int max) {
    while (true) {
        const std::string text = readLine(prompt);

        try {
            std::size_t used = 0;
            const int value = std::stoi(text, &used);

            // require the whole input to be a valid menu number within the allowed range.
            if (used == text.size() && value >= min && value <= max) {
                return value;
            }
        } catch (...) {
            // the message below handles all malformed numeric input consistently.
        }

        std::cout << "[SYSTEM] Enter a number from " << min << " to " << max << ".\n";
    }
}

///// USERNAME CHARACTER RULES /////
bool validateUsername(const std::string& username) {
    if (username.size() < USERNAME_MIN_LENGTH || username.size() > USERNAME_MAX_LENGTH) {
        return false;
    }

    // unsigned char keeps values valid for std::isalnum even when char is signed.
    for (unsigned char character : username) {
        if (!std::isalnum(character) && character != '_' && character != '-' && character != '.') {
            return false;
        }
    }

    return true; // valid username
}

// PASSWORD LENGTH AND CONTROL-CHARACTER CHECKS
bool validatePassword(const std::string& password) {
    if (password.size() < PASSWORD_MIN_LENGTH || password.size() > PASSWORD_MAX_LENGTH) {
        return false;
    }

    // examine each password byte for control characters.
    for (unsigned char character : password) {
        if (std::iscntrl(character)) {
            return false;
        }
    }

    return true;
}

// DEMO ID - EXACTLY FOUR DIGITS
bool validateIDLastFour(const std::string& idLastFour) {
    if (idLastFour.size() != 4) {
        return false;
    }

    for (unsigned char character : idLastFour) {
        if (!std::isdigit(character)) {
            return false;
        }
    }

    return true;
}

bool validateIDType(int idType) {
    return idType >= 1 && idType <= 3;
}

///// MONEY INPUT - DECIMAL TEXT TO INTEGER CENTS /////
bool parseMoneyToCents(const std::string& text, std::int64_t& centsOut) {
    if (text.empty()) {
        return false;
    }

    std::string dollarsPart;
    std::string centsPart;
    bool seenDecimal = false;

    for (char character : text) {
        if (character == '.') {
            if (seenDecimal) {
                return false;
            }
            seenDecimal = true;
            continue;
        }

        if (!std::isdigit(static_cast<unsigned char>(character))) {
            return false;
        }

        if (seenDecimal) {
            centsPart += character;
        } else {
            dollarsPart += character;
        }
    }

    if (dollarsPart.empty()) {
        dollarsPart = "0";
    }

    if (centsPart.size() > 2) {
        return false;
    }

    while (centsPart.size() < 2) {
        centsPart += '0';
    }

    try {
        const std::int64_t dollars = std::stoll(dollarsPart);
        const std::int64_t cents = centsPart.empty() ? 0 : std::stoll(centsPart);

        // check for overflow before converting whole dollars to integer cents.
        if (dollars > (std::numeric_limits<std::int64_t>::max() - cents) / 100) {
            return false;
        }

        centsOut = dollars * 100 + cents;
        return true;
    } catch (...) {
        return false;
    }
}

// KEEP ASKING UNTIL THE MONEY INPUT IS VALID
std::int64_t readMoneyCents(const std::string& prompt, bool allowZero) {
    while (true) {
        const std::string text = readLine(prompt);
        std::int64_t cents = 0;

        // zero is accepted only when allowZero is true; transactions require positive money.
        if (parseMoneyToCents(text, cents) && (allowZero ? cents >= 0 : cents > 0)) {
            return cents;
        }

        std::cout << "[SYSTEM] Enter a valid non-negative dollar amount with at most two decimal places.\n";
    }
}

// FORMAT INTEGER CENTS AS DOLLARS
std::string formatMoney(std::int64_t cents) {
    std::ostringstream output;
    output << '$' << (cents / 100) << '.' << std::setw(2) << std::setfill('0') << (cents % 100);
    return output.str();
}

} // namespace InputHandling
