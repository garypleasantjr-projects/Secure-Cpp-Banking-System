#ifndef INPUT_HANDLING_H
#define INPUT_HANDLING_H

#include <cstdint>
#include <string>

// These reusable validation/input helpers live in their own namespace
namespace InputHandling {

constexpr std::size_t USERNAME_MIN_LENGTH = 3;
constexpr std::size_t USERNAME_MAX_LENGTH = 24;
constexpr std::size_t PASSWORD_MIN_LENGTH = 12;
constexpr std::size_t PASSWORD_MAX_LENGTH = 128;

std::string readLine(const std::string& prompt);
int readChoice(const std::string& prompt, int min, int max);
// The optional flag lets account creation accept a zero starting balance
std::int64_t readMoneyCents(const std::string& prompt, bool allowZero = false);

bool validateUsername(const std::string& username);
bool validatePassword(const std::string& password);
bool validateIDLastFour(const std::string& idLastFour);
bool validateIDType(int idType);

bool parseMoneyToCents(const std::string& text, std::int64_t& centsOut);
std::string formatMoney(std::int64_t cents);

} // namespace InputHandling

#endif
