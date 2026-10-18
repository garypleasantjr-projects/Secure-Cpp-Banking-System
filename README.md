# Gary's Secure Banking Demo

A C++ secure-coding project that grew from a basic console banking assignment into a small security-focused application with password hashing, persistence, input validation, audit logging, login throttling, and safer data handling.

> **Important:** This is an educational project, **not a real banking system**. Do not enter real SSNs, ITINs, EINs, passwords, or financial data.

## Project Goal

The goal is to show how ordinary application features become security-engineering decisions. The program lets a user:

- create an account
- log in
- view a balance
- deposit and withdraw funds
- change a password
- persist accounts to disk
- record security-relevant audit events

The interesting part is not the banking menu itself. The interesting part is how the code handles **credentials, untrusted input, persistence, authentication, error cases, and sensitive data**.

---

## Progression Story: From the First Version to the Finished Version

### Stage 1 - Basic banking structure

The original version started with a simple console menu in `main.cpp` for account creation and login. It already separated responsibilities into files such as:

- `Account`
- `AccountManager`
- `Crypto`
- `InputHandling`

That was a good architectural starting point because it avoided putting everything into one giant `main.cpp`.

### Stage 2 - Security-aware credential storage

The early project already moved beyond plaintext passwords by using OpenSSL PBKDF2-HMAC-SHA256, a random salt, and byte-vector storage for password hashes.

The original code also contained TODOs for:

- timing-safe password comparison
- stronger exception handling
- loading and saving accounts correctly
- completing login flow
- improving ownership/pointer decisions

Those TODOs became the main roadmap for Version 2.

### Stage 3 - Finished authentication and persistence

This version completes the account lifecycle:

1. Account is created.
2. A unique random salt is generated.
3. The password is converted into a PBKDF2-HMAC-SHA256 derived value.
4. Salt, hash, work factor, balance, ID type, and only the **last four demo ID digits** are stored.
5. On login, the same password derivation is repeated with the saved salt and saved work factor.
6. The result is compared using OpenSSL's constant-time comparison helper.

A major bug from the unfinished design was also corrected: **loading an existing account must preserve its original salt**. Generating a new salt when loading an old password hash would make the saved password impossible to verify.

### Stage 4 - Safer application design

The project was upgraded with several decisions that are easy to understand but meaningful from a security perspective:

- Money is stored as **integer cents**, not `double`, so normal floating-point rounding does not affect balances.
- The program stores only the **last four digits** of the fictional demo identifier instead of a full SSN/ITIN/EIN.
- Passwords are never saved in plaintext.
- Passwords are not written to the audit log.
- Authentication errors are intentionally generic.
- Repeated failed logins cause a temporary lockout.
- Invalid or malformed database records fail closed instead of being silently accepted.
- The application refuses to overwrite a database it cannot safely parse.
- Password changes create a fresh salt and use the current work factor.
- The program uses `std::string` and `std::vector` rather than manual fixed-size C buffers for user-controlled text.

### Stage 5 - Engineering structure

The finished project adds:

- CMake build configuration
- automated core and persistence/security regression tests
- GitHub Actions CI for build-and-test verification
- security documentation
- a learning guide explaining why each improvement was made
- audit logging
- a versioned database format

This turns the project from "code that works" into a portfolio project that can be discussed in an interview.

---

## Security Features

### Password storage

- PBKDF2-HMAC-SHA256 through OpenSSL
- 600,000 iterations for newly created/changed passwords
- 16-byte cryptographically random salt per password
- 32-byte derived key
- constant-time password-hash comparison with `CRYPTO_memcmp`
- work factor stored with each account so the scheme can be upgraded later

### Authentication defenses

- generic failure message for incorrect username/password
- expensive dummy password hashing when the username does not exist, reducing the obvious timing difference between known and unknown usernames
- five failed attempts trigger a 30-second in-memory lockout
- password changes require the current password

### Input and data handling

- usernames use a small allowlist of characters
- passwords allow long passphrases instead of requiring arbitrary symbol rules
- balances are parsed into integer cents
- only four fictional/demo identifier digits are stored
- malformed database fields are rejected

### Logging

The audit log records events such as:

- account creation
- login success/failure
- temporary login blocking
- deposits and withdrawals
- password changes
- logout

It intentionally does **not** record passwords or full identification numbers.

---

## OWASP / CWE Security Mapping

This is an educational mapping, not a claim that the program is "OWASP certified" or production-ready.

The current OWASP Top 10:2025 includes categories such as Cryptographic Failures, Authentication Failures, Security Logging & Alerting Failures, Insecure Design, and Mishandling of Exceptional Conditions. This project directly demonstrates controls related to several of them.

| Reference | How this project responds |
|---|---|
| **OWASP A04:2025 - Cryptographic Failures** | Passwords are salted and processed with PBKDF2-HMAC-SHA256 rather than stored as plaintext or fast unsalted hashes. |
| **OWASP A06:2025 - Insecure Design** | Sensitive demo ID collection is minimized, responsibilities are separated, balances use integer cents, and dangerous failure behavior is reduced. |
| **OWASP A07:2025 - Authentication Failures** | Password hashing, generic login errors, current-password verification, and temporary throttling/lockout are used. |
| **OWASP A09:2025 - Security Logging & Alerting Failures** | Security-relevant events are logged. The project demonstrates logging, but it does **not** implement a real monitoring/alerting system. |
| **OWASP A10:2025 - Mishandling of Exceptional Conditions** | Invalid input, malformed database records, cryptographic failures, and file errors are handled deliberately instead of being ignored. |
| **CWE-20 - Improper Input Validation** | Usernames, identifiers, menu choices, and money input are validated before use. |
| **CWE-200 - Exposure of Sensitive Information** | Full demo identifiers are not persisted, plaintext passwords are never stored, and secrets are omitted from logs. |
| **CWE-306 - Missing Authentication for Critical Function** | Account operations are only reachable after authentication. |
| **CWE-787 / CWE-125 family - Out-of-bounds memory access** | User-controlled text is handled with standard C++ containers instead of manual raw-buffer copying. This reduces risk but does not make C++ memory-safe. |

### What this project does NOT claim to solve

OWASP Top 10 is primarily a web-application awareness standard. This console application does not use SQL, HTML, HTTP, a browser, or a web framework, so claims about preventing SQL injection, XSS, CSRF, SSRF, or web access-control vulnerabilities would be misleading.

The local database is also not encrypted or cryptographically authenticated. A user who can directly modify local files can tamper with the database. That is a deliberate remaining limitation documented for future work.

Authoritative references used for the security design discussion:

- OWASP Top 10:2025: https://top10.owasp.org/2025/
- OWASP Password Storage Cheat Sheet: https://cheatsheetseries.owasp.org/cheatsheets/Password_Storage_Cheat_Sheet.html
- MITRE 2025 CWE Top 25: https://cwe.mitre.org/top25/archive/2025/2025_cwe_top25
- NIST Digital Identity Guidelines: https://pages.nist.gov/800-63-4/

---

## Project Structure

```text
Secure-Cpp-Banking-System/
├── .github/
│   └── workflows/
│       └── ci.yml
├── CMakeLists.txt
├── README.md
├── SECURITY_DESIGN.md
├── LEARNING_GUIDE.md
├── .gitignore
├── data/
│   └── .gitkeep
├── src/
│   ├── main.cpp
│   ├── Account.h
│   ├── Account.cpp
│   ├── AccountManager.h
│   ├── AccountManager.cpp
│   ├── AuditLogger.h
│   ├── AuditLogger.cpp
│   ├── BankingUI.h
│   ├── BankingUI.cpp
│   ├── Crypto.h
│   ├── Crypto.cpp
│   ├── InputHandling.h
│   └── InputHandling.cpp
└── tests/
    └── test_core.cpp
```

---

## Build Requirements

- C++17 compiler
- CMake 3.16+
- OpenSSL development libraries

### Linux / macOS style build

```bash
cmake -S . -B build
cmake --build build
./build/secure_bank
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

On macOS with Homebrew OpenSSL, CMake may need an explicit OpenSSL path depending on the machine.

---

## Database Format

The file begins with:

```text
SECURE_BANK_DB_V1
```

Each account record stores:

```text
username|salt_hex|password_hash_hex|iterations|balance_cents|id_type|id_last_four
```

Example structure only:

```text
demo.user|<salt>|<hash>|600000|12500|1|1234
```

No plaintext password is stored.

---

## Why the code still uses an `Account*`

The original project had a TODO saying the pointer returned by `findAccount()` should become a smart pointer.

In this design, `AccountManager` owns the `Account` objects inside its `std::vector<Account>`. The returned pointer is only a **temporary, non-owning pointer** to an object that the manager still owns. A `std::unique_ptr` would incorrectly imply that the caller owns and should eventually destroy the account.

So keeping a normal pointer here is intentional and is actually simpler ownership design.

---

## Future Improvements

Good next steps, after fully understanding this version:

1. Replace the flat-file database with SQLite using prepared statements.
2. Encrypt/authenticate sensitive local data at rest.
3. Add a proper role/authorization model.
4. Add transaction history with integrity checks.
5. Add unit tests for database loading and lockout behavior.
6. Add a network/API layer only after the local security model is understood.
7. Consider Argon2id where an appropriate library/deployment environment is available.
8. Add dependency scanning and static analysis to the existing GitHub Actions CI workflow.

---

## Portfolio Summary

**Secure C++ Banking / Authentication Demo**  
C++17, OpenSSL, PBKDF2-HMAC-SHA256, secure random salts, constant-time comparison, input validation, file persistence, audit logging, authentication throttling, integer-money handling, CMake, and automated testing.

The project demonstrates how a basic programming assignment can be iteratively redesigned around secure coding and application-security principles.
