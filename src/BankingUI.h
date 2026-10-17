#pragma once

class AccountManager;
class AuditLogger;

namespace BankingUI {
    void run(AccountManager& manager, AuditLogger& logger);
}
