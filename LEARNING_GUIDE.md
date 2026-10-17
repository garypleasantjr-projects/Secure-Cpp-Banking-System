# Learning Guide: Understanding the Upgrade

This file is meant to make the finished project understandable rather than turning it into code you cannot explain.

## Recommended reading order

1. `src/main.cpp`
2. `src/InputHandling.cpp`
3. `src/Account.h` and `src/Account.cpp`
4. `src/Crypto.cpp`
5. `src/AccountManager.cpp`
6. `src/AuditLogger.cpp`
7. `tests/test_core.cpp`

That order follows the program from the user interface down toward the lower-level security pieces.

---

## 1. `main.cpp`: program flow

This file should feel the most familiar.

It has three major flows:

- create account
- login
- logged-in account menu

The important improvement is that `main.cpp` mostly coordinates other classes instead of implementing cryptography or persistence itself.

### What to understand

Look for:

```cpp
Account* account = manager.authenticateUser(username, password, statusMessage);
```

The `AccountManager` owns the account objects. `main.cpp` only receives a temporary pointer to the authenticated account.

---

## 2. `InputHandling.cpp`: never trust raw input

Your original project already recognized input validation as its own concern. This version finishes that idea.

Instead of switching back and forth between `cin >>` and `getline()`, most input is read as a **string first**, then parsed.

That avoids a lot of leftover-newline and fail-state problems.

### Money parsing

The program converts:

```text
125.50
```

into:

```text
12550 cents
```

Why? `double` cannot represent every decimal value exactly. Integer cents are simpler for a learning project involving money.

---

## 3. `Account.cpp`: one account's data and behavior

The account class owns:

- username
- salt
- password hash
- password iteration count
- balance in cents
- ID type
- last four demo ID digits

The key method is:

```cpp
bool Account::authenticate(const std::string& attemptedPassword) const
```

It never "decrypts" the saved password. Instead it hashes the attempted password using the same saved salt/work factor and compares the result to the saved hash.

### Why loading uses a second constructor

When creating a new account, the program needs to generate a salt and password hash.

When loading an account, those values **already exist** and must be restored exactly.

The unfinished version generated a new salt during load. That would break authentication because the original password hash was created with a different salt.

---

## 4. `Crypto.cpp`: the low-level security work

The main PBKDF2 call is:

```cpp
PKCS5_PBKDF2_HMAC(... EVP_sha256() ...)
```

Inputs include:

- password bytes
- salt bytes
- iteration count
- output length

The generated password value is a derived key, not reversible ciphertext.

### Constant-time comparison

The original source had a TODO for a timing-safe operation.

The finished version uses:

```cpp
CRYPTO_memcmp(...)
```

Normal equality comparisons can sometimes stop as soon as they find a mismatch. Constant-time comparison is designed to avoid making the comparison duration depend on where the mismatch occurs.

---

## 5. `AccountManager.cpp`: ownership, persistence, and login policy

This is the most advanced file, so do not try to memorize it all at once.

It handles:

- the vector of accounts
- duplicate usernames
- loading and saving
- failed-login tracking
- temporary lockout

### Why the pointer is not a smart pointer

The manager owns this container:

```cpp
std::vector<Account> accounts;
```

A pointer returned by `findAccount()` does not own the account. It only points at an account owned by the manager.

Using `std::unique_ptr` for the return value would imply the caller became the owner, which would be the wrong model.

This is an important C++ lesson: **smart pointers are for ownership, not just because raw pointers exist.**

### Load into a temporary vector first

The loader creates a temporary collection and validates every record before replacing the live account list.

That prevents this situation:

1. three accounts load successfully
2. line four is corrupt
3. program keeps a half-loaded database

Instead, loading fails as a single operation.

---

## 6. Login throttling

Each username gets a small `LoginAttemptState` containing:

- number of failures
- lockout expiration time

After five failures, login is blocked for 30 seconds.

This is deliberately simple. A production system would probably store rate-limit state outside the application process and combine account, IP, device, and risk signals.

---

## 7. `AuditLogger.cpp`: useful logs without secrets

Security logging is useful only if the logs do not create a second secret database.

The logger records event types and usernames but intentionally does not record:

- plaintext passwords
- password hashes
- full IDs

That distinction is worth being able to explain in an interview.

---

## 8. Automated tests

`tests/test_core.cpp` uses simple `assert()` calls instead of a large testing framework.

That keeps the tests understandable while still demonstrating that the project checks:

- validation rules
- money parsing
- password hashing
- constant-time equality wrapper behavior
- account authentication
- deposit/withdrawal behavior
- password changes

---

## Original idea -> finished concept

| Original concept/TODO | Finished implementation |
|---|---|
| PBKDF2 with OpenSSL | Kept and raised to current project work factor |
| Random salt | Kept and correctly persisted/reloaded |
| `attemptedHash == passwordHash` | `CRYPTO_memcmp` wrapper |
| Full numeric ID | Last four demo digits only |
| `double balance` | `int64_t` cents |
| Incomplete file load/save | Versioned validated persistence |
| Empty `catch` block | Specific exceptions and safe failure paths |
| Basic login | Generic errors + throttling/lockout |
| TODO smart pointer | Documented non-owning pointer design |
| No account session menu | Completed balance/deposit/withdraw/password flow |
| No security event record | Audit logger |
| No tests/build project | CMake + automated core tests |

---

## What you should be able to explain before putting this on a resume

You do **not** need to memorize OpenSSL internals. You should be able to explain:

- why passwords are hashed instead of encrypted
- what a salt does
- what the PBKDF2 iteration count does
- why the saved salt must be reused for verification
- why constant-time comparison is used
- why money is stored as cents
- why only last-four demo ID digits are kept
- why malformed database records cause a safe failure
- why login throttling exists
- why the pointer returned by the manager is non-owning
- what security limitations still remain

If you can explain those clearly, the project is genuinely yours to discuss even though the finished version is more polished than the draft.
