#ifndef AUDIT_LOGGER_H
#define AUDIT_LOGGER_H

#include <string>

// encapsulate the path and function used for security-event logging.
// AUDIT LOGGER - SECURITY EVENTS
class AuditLogger {
private:
    std::string logFileName;

///// LOGGER INTERFACE /////
public:
    // explicit prevents a filename from being converted implicitly into a logger.
    explicit AuditLogger(const std::string& logFileName);
    void log(const std::string& event, const std::string& username = "") const;
};

#endif
