#pragma once

// Forward-declare classes needed by reference without including their full headers
class AccountManager;
class AuditLogger;

namespace BankingUI {
    // Share the existing manager and logger with the user-interface module
    void run(AccountManager& manager, AuditLogger& logger);
}
