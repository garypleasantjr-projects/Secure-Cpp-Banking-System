#pragma once

// forward-declare classes needed by reference without including their full headers.
class AccountManager;
class AuditLogger;

// BANKING UI - PUBLIC ENTRY POINT
namespace BankingUI {
    // share the existing manager and logger with the user-interface module.
    void run(AccountManager& manager, AuditLogger& logger);
}
