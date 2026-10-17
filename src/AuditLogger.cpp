#include "AuditLogger.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

// AUDIT LOGGER CONSTRUCTOR
AuditLogger::AuditLogger(const std::string& logFileName)
    : logFileName(logFileName) {}

// WRITE A TIME-STAMPED EVENT TO THE LOG
void AuditLogger::log(const std::string& event, const std::string& username) const {
    // append to the log instead of overwriting earlier events.
    std::ofstream logFile(logFileName, std::ios::app);
    if (!logFile) {
        return; // Logging failure must not crash the banking demo.
    }

    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};
// choose the platform's supported thread-safe local-time conversion.
#if defined(_WIN32)
    localtime_s(&localTime, &nowTime);
#else
    localtime_r(&nowTime, &localTime);
#endif

    logFile << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
            << " | " << event;

    // include the username only when the event relates to an account.
    if (!username.empty()) {
        logFile << " | user=" << username;
    }

    logFile << '\n'; // end this log entry with a newline
}
