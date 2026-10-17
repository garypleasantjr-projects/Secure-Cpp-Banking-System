#ifndef AUDIT_LOGGER_H
#define AUDIT_LOGGER_H

#include <string>

class AuditLogger {
private:
    std::string logFileName;

public:
    explicit AuditLogger(const std::string& logFileName);
    void log(const std::string& event, const std::string& username = "") const;
};

#endif
