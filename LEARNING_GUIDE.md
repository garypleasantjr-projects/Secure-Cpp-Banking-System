# Learning Guide — Secure C++ Banking System

## Purpose

This document explains how the project evolved from my original March prototype into the current portfolio version, what each major design choice is meant to teach, and what I would improve next.

The goal is not to present this demo as a production banking platform. The goal is to show a progression in **C++, secure coding, authentication, cryptography, persistence, testing, and software-engineering judgment**.

---

## 1. Progression from the original version

### March prototype

The original project already had the main architecture:

- `Account`
- `AccountManager`
- `Crypto`
- `InputHandling`
- `main.cpp`

It also already attempted several security-conscious ideas:

- password hashing instead of plaintext storage
- PBKDF2-HMAC-SHA256 through OpenSSL
- random salts
- username/password validation
- account creation and login
- deposit and withdrawal logic
- a planned persistence layer

The early source also contained TODOs for areas I knew needed improvement, including:

- timing-safe password comparison
- correct account loading and saving
- better exception handling
- finishing the login/session flow
- deciding when a raw pointer or smart pointer made sense

The current version is the result of finishing those ideas and then adding a stronger security-engineering layer around them.

### Current portfolio version

The current version adds or completes:

- a full account lifecycle
- a complete logged-in menu
- correct password verification after database reload
- per-password random salts
- PBKDF2 work-factor storage
- constant-time hash comparison
- password changes with a new salt
- integer-based money storage
- input validation
- login throttling / temporary lockout
- audit logging
- versioned file persistence
- fail-closed database loading
- CMake
- automated tests
- OWASP/CWE-oriented security documentation

The important progression is not simply “more code.” It is the move from making features work to asking:

> What input is untrusted?  
> What data is sensitive?  
> What assumptions can fail?  
> How should failure behave?  
> What should be logged?  
> What should never be stored?  
> Which security claims can this program actually support?

---

## 2. How to study the code

I would study the repository in this order.

### Step 1 — `InputHandling`

Start here because the functions are small and easy to trace.

Understand:

- why menu input is read as a string first
- why parsing and validation are separate concerns
- how usernames are allowlisted
- why control characters are rejected from passwords
- why money is parsed into integer cents
- why invalid input loops instead of falling into business logic

Key idea:

> Validate untrusted data before the rest of the application relies on it.

### Step 2 — `Account`

Then study the object that owns one user's state.

Understand:

- the difference between the “new account” constructor and the “load existing account” constructor
- why a loaded account must reuse its saved salt
- why the password hash and salt are stored as byte vectors
- why balances use `std::int64_t` cents instead of `double`
- how `authenticate()` derives a candidate hash and compares it
- why password changes generate a new salt

Key idea:

> The `Account` class holds security-sensitive state and should preserve its invariants.

### Step 3 — `Crypto`

Study the OpenSSL code after the `Account` flow makes sense.

Understand:

- what a salt is
- why each password gets a unique salt
- what PBKDF2 does
- what the iteration/work factor does
- why a password hash is not encryption
- why `CRYPTO_memcmp` is used instead of normal equality for the derived key
- how binary salt/hash data is converted to hexadecimal for persistence

The project currently uses PBKDF2-HMAC-SHA256 with 600,000 iterations. OWASP currently prefers Argon2id for new general-purpose password-storage designs, while its Password Storage Cheat Sheet gives PBKDF2-HMAC-SHA256 with 600,000+ iterations as its FIPS-oriented PBKDF2 guidance.

Key idea:

> Password storage should use a deliberately slow password-hashing/KDF design, unique salts, and an upgradeable work factor.

### Step 4 — `AccountManager`

This is where more of the engineering decisions come together.

Understand:

- how accounts are stored
- how duplicate usernames are detected
- how authentication state is tracked
- what the temporary lockout does
- why a dummy PBKDF2 calculation is performed for an unknown username
- how the database is parsed
- why malformed records cause the load to fail
- why the program loads into a temporary vector before replacing the live account list
- how save failures cause in-memory operations to be rolled back

Key idea:

> Security is often about safe state transitions, not just cryptographic functions.

### Step 5 — `AuditLogger`

Understand:

- which security-relevant events are logged
- why passwords and full identifiers must not be logged
- why logs are useful for investigations
- why this demo's local append-only text log is not tamper-resistant

Key idea:

> Logging is a security control only when the log contains useful events without leaking secrets.

### Step 6 — `main.cpp`

Study `main.cpp` last.

By this point, it should read mostly as orchestration:

- display menus
- collect input
- call validation helpers
- call account/account-manager operations
- display results

That is preferable to placing cryptography, parsing, persistence, and business logic directly inside one giant `main()` function.

---

## 3. Major security decisions I should be able to explain

### Passwords are not stored in plaintext

The program stores a derived value produced by PBKDF2-HMAC-SHA256.

If the database is copied, there is no plaintext password field to read directly.

### Every password has its own salt

A random salt makes identical passwords produce different stored derived values.

This prevents a single precomputed hash lookup from working across all accounts.

### The work factor is stored with the account

The database stores the iteration count with each credential.

That makes future migration possible because an older account can still be verified using its original work factor.

### Password comparison uses a constant-time helper

`CRYPTO_memcmp` reduces data-dependent timing differences during comparison of equal-length derived keys.

It does not make the entire login path perfectly constant-time, but it avoids using an ordinary early-exit byte comparison for the secret-derived value.

### Money uses integer cents

`double` is useful for many numerical problems, but decimal financial values can produce rounding surprises in binary floating-point.

Representing `$12.34` as `1234` cents makes balance arithmetic easier to reason about.

### The program minimizes identity data

The original design asked for a full SSN/ITIN/EIN-style value.

The current demo stores only four fictional identifier digits and explicitly warns against entering real identity information.

This demonstrates data minimization: if the application does not need sensitive data, do not collect it.

### Database loading fails closed

A malformed database is not silently treated as trustworthy account state.

The loader validates the version, field count, hash/salt sizes, identifiers, balances, and duplicate usernames before replacing the live account vector.

### Authentication errors are intentionally generic

A normal bad login reports that the username or password is incorrect rather than revealing which field was valid.

### Repeated failures are throttled

Five failed attempts trigger a temporary in-memory lockout.

This is an educational anti-guessing control, not a production authentication service.

### The returned `Account*` is non-owning

`AccountManager` owns the account objects inside its `std::vector<Account>`.

A `std::unique_ptr<Account>` returned to the caller would incorrectly suggest that the caller owns and should destroy that account.

The raw pointer means:

> “Here is a temporary reference to an object owned somewhere else.”

That is a valid use of a raw pointer as long as the owner and lifetime rules remain clear.

---

## 4. OWASP / CWE connection

This project uses OWASP and CWE as **reference frameworks**, not as a claim that the application “solves the OWASP Top 10.”

Relevant examples include:

- **OWASP A04:2025 — Cryptographic Failures**  
  Passwords are not stored as plaintext or fast unsalted hashes.

- **OWASP A06:2025 — Insecure Design**  
  The project minimizes stored identity data and separates security-sensitive responsibilities.

- **OWASP A07:2025 — Authentication Failures**  
  Password verification, generic failure messages, password-change verification, and login throttling are demonstrated.

- **OWASP A09:2025 — Security Logging and Alerting Failures**  
  Authentication and account-security events are recorded, although this demo does not provide centralized monitoring or alerting.

- **OWASP A10:2025 — Mishandling of Exceptional Conditions**  
  File, parsing, cryptographic, and malformed-input failures are handled intentionally.

- **CWE-20 — Improper Input Validation**  
  Usernames, menu choices, money values, identifiers, and persisted fields are validated.

- **CWE-200 — Exposure of Sensitive Information**  
  Plaintext passwords and full identity numbers are intentionally excluded from persistence and logs.

The application has no HTTP layer, browser, SQL database, or web framework, so claims about preventing XSS, CSRF, SSRF, or SQL injection would not be appropriate.

---

## 5. Known limitations and “Do Later” engineering improvements

These are intentionally documented as future work rather than hidden. They are useful follow-up exercises because each one connects to a real software-engineering concept.

### Do later 1 — Make database writes atomic

**Current behavior:** `saveToDatabaseFile()` opens the real database with `std::ios::trunc` and writes directly to it.

**Problem:** if the write fails after truncation, the old database may already be damaged or partially replaced. Rolling back the in-memory balance does not restore the previous on-disk file.

**Improvement:** write the complete new database to a temporary file, verify/flush it, then atomically rename/replace the real database.

**What this teaches:** crash consistency, transactional updates, and safe persistence.

### Do later 2 — Require full numeric-field parsing

**Current behavior:** database loading uses `std::stoi` / `std::stoll` without checking how many characters were consumed.

**Problem:** a field such as `600000junk` can be parsed as the numeric prefix `600000` instead of being rejected as fully malformed.

**Improvement:** capture the parsed-character count and require it to equal the entire string length, or use `std::from_chars`.

**What this teaches:** strict parser design and input canonicalization.

### Do later 3 — Check balance arithmetic for integer overflow

**Current behavior:** `deposit()` adds directly to `balanceCents`.

**Problem:** an extremely large deposit could overflow `std::int64_t`.

**Improvement:** check `amountCents <= INT64_MAX - balanceCents` before addition.

**What this teaches:** numeric boundary validation and integer-overflow prevention.

### Do later 4 — Enforce invariants inside the domain classes

**Current behavior:** the UI and loader validate usernames, password length, ID type, and ID digits before constructing or modifying objects.

**Problem:** another future caller could use `Account` directly and bypass those checks.

**Improvement:** validate important invariants inside `Account` / `AccountManager` as well as at the UI boundary.

**What this teaches:** defense in depth and object invariants.

### Do later 5 — Improve authentication rate-limit state

**Current behavior:** `loginAttempts` is an in-memory map keyed by the submitted username.

**Problems:**

- the map can grow if many unique usernames are submitted
- lockout state disappears when the process restarts
- a username-only lockout model can become a denial-of-service concern in a real network service

**Improvement:** bound/expire the map, use a better rate-limiting design, and persist or centralize the state if the application ever becomes networked.

**What this teaches:** availability, abuse resistance, and state lifecycle management.

### Do later 6 — Revisit dummy-hash timing when credential versions differ

**Current behavior:** unknown usernames use the current PBKDF2 work factor for dummy work.

**Problem:** if old accounts later retain different historical iteration counts, the runtime of known and unknown-user paths may differ again.

**Improvement:** design the dummy work and credential-upgrade flow together.

**What this teaches:** side-channel thinking and versioned authentication schemes.

### Do later 7 — Improve password entry and memory handling

**Current behavior:** passwords are entered through normal terminal input and temporarily exist in `std::string`.

**Limitations:**

- the password is visible while typed
- plaintext password bytes may remain in process memory until normal memory reuse occurs

**Improvement:** use platform-appropriate hidden terminal input and investigate secure-memory handling where justified.

**What this teaches:** secrets can leak outside the database layer too.

### Do later 8 — Strengthen persistence integrity

**Current behavior:** salt/hash data is protected for password verification, but the database file itself is not authenticated.

**Problem:** someone with local write access can modify balances, identifier fields, iteration counts, or account records.

**Improvement:** move to SQLite or another structured store and add an appropriate integrity/authentication design for the threat model.

**What this teaches:** password hashing does not provide database integrity.

### Do later 9 — Make audit logging more robust

**Current behavior:** logging failure is intentionally non-fatal.

**Limitations:**

- the application does not alert the user/operator if logging fails
- logs can be edited by someone with local file access
- logs are not centralized or monitored

**Improvement:** expose logging health, consider integrity protection, and integrate monitoring if the project becomes a service.

**What this teaches:** logging, monitoring, and alerting are different controls.

### Do later 10 — Expand automated testing

The current test covers core validation, cryptography, account authentication, deposits/withdrawals, and password changes.

Add tests for:

- database round trips
- malformed database lines
- partial/failed saves
- login lockout behavior
- duplicate accounts
- integer-overflow boundaries
- audit logging
- password-work-factor migration

Also replace or supplement `assert()` with a small test framework or explicit test checks so Release builds cannot disable the test assertions through `NDEBUG`.

**What this teaches:** security controls need regression tests.

### Do later 11 — Add CI

Add a GitHub Actions workflow that:

1. configures CMake
2. builds the project
3. runs `ctest`
4. optionally runs static analysis

**What this teaches:** repeatable verification and software supply-chain hygiene.

---

## 6. Repository / Git skills learned

This project also became my introduction to a more realistic GitHub workflow.

The repository uses:

- a `src/` directory for production source code
- a `tests/` directory for test code
- CMake for reproducible builds
- `.gitignore` for generated/runtime files
- Markdown documentation
- Git branches
- commits
- a pull request
- merge into `main`

Those practices are separate from C++ syntax, but they are part of working on software in a team.

---

## 7. What I should be able to explain in an interview

Before using this project heavily in an interview, I should be able to answer these without reading the README:

1. Why hash passwords instead of encrypting them?
2. What does a salt do?
3. Why use PBKDF2 instead of plain SHA-256?
4. What does the work factor accomplish?
5. Why store the iteration count per account?
6. Why use `CRYPTO_memcmp`?
7. Why must the original salt be preserved after reloading an account?
8. Why use integer cents instead of `double`?
9. What is the difference between validation and sanitization?
10. Why does the database loader fail closed?
11. Why is `Account*` non-owning here?
12. What can invalidate a pointer into a `std::vector`?
13. Why is the current pointer usage safe in the present single-session flow, and what future changes could make it unsafe?
14. What does the login lockout protect against?
15. What are the limitations of that lockout?
16. What does the audit log record, and what should never be logged?
17. Why is the database still tamperable even though passwords are hashed?
18. Why is direct truncating file output weaker than an atomic temp-file replacement?
19. What security properties does this project *not* provide?
20. What would I build next if this became a networked application?

If I can explain those clearly, I understand the important engineering ideas behind the project rather than only knowing that the program runs.

---

## 8. Suggested next progression

A good next sequence would be:

1. understand and explain every current module
2. fix the strict numeric parsing issue
3. add overflow checks
4. make file saving atomic
5. expand tests
6. add GitHub Actions CI
7. move persistence to SQLite
8. add a small API/service layer
9. threat-model the networked version
10. add API authentication/authorization and application-security testing

That progression naturally connects this C++ project to later work in:

- application security
- product security
- security engineering
- secure software development
- network software security
- vulnerability research

---

## 9. Authoritative references

- OWASP Top 10:2025  
  https://top10.owasp.org/2025/

- OWASP Password Storage Cheat Sheet  
  https://cheatsheetseries.owasp.org/cheatsheets/Password_Storage_Cheat_Sheet.html

- MITRE CWE  
  https://cwe.mitre.org/

- NIST Digital Identity Guidelines  
  https://pages.nist.gov/800-63-4/

---

## Final takeaway

The strongest part of this project is the progression.

It began as a C++ banking/account exercise with security ideas already present in the design. The current version turns those ideas into a complete educational application and makes the remaining limitations explicit.

The next stage is not adding random features. It is improving the places where **correctness, security, persistence, testing, and maintainability meet**.
