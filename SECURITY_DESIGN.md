# Security Notes and Vulnerability Mapping

## Scope

This repository is an educational local C++ application. It is **not a production banking platform** and should never be used with real financial or identity data.

The purpose of this document is to connect concrete code decisions to recognized software-security references without claiming protections the project does not actually implement.

## Recognized References

- OWASP Top 10:2025
- OWASP Password Storage Cheat Sheet
- MITRE 2025 CWE Top 25 Most Dangerous Software Weaknesses
- NIST SP 800-63 Digital Identity Guidelines

## Controls implemented

### 1. Credential protection

**Threat:** plaintext password disclosure, weak offline password storage, reusable hashes.

**Controls:**

- PBKDF2-HMAC-SHA256
- unique random salt per password
- 600,000 iterations for new/changed passwords
- password work factor stored with each account
- password hashes compared with `CRYPTO_memcmp`
- plaintext passwords never written to disk or logs

**Relevant references:** OWASP A04 Cryptographic Failures, A07 Authentication Failures; CWE credential-protection weaknesses.

### 2. Repeated login guessing

**Threat:** unlimited repeated password guesses.

**Controls:**

- five failed attempts trigger a temporary 30-second lockout
- missing usernames still perform a dummy PBKDF2 operation to reduce easy timing-based username discovery
- generic username/password failure messages

**Limit:** Lockout state is only in memory and resets when the process restarts.

### 3. Improper input validation

**Threat:** malformed values reaching business logic or persistence code.

**Controls:**

- allowlisted username characters
- bounded username/password lengths
- exact four-digit demo identifier validation
- strict money parser
- bounded menu choices
- database field validation during load

**Relevant references:** CWE-20 Improper Input Validation; OWASP A10 Mishandling of Exceptional Conditions.

### 4. Sensitive-data minimization

**Threat:** unnecessary storage of identity data.

**Control:** The original design's full SSN/ITIN/EIN field was replaced with **last four digits only**, and the UI explicitly tells users to enter fictional/demo values.

**Relevant references:** OWASP A06 Insecure Design; CWE-200 Exposure of Sensitive Information.

### 5. Financial data correctness

**Threat:** floating-point rounding errors in balances.

**Control:** Monetary values use integer cents (`std::int64_t`) rather than `double`.

This is primarily a correctness/integrity improvement rather than a direct OWASP Top 10 mitigation.

### 6. Security logging

**Threat:** no record of authentication or security-relevant events.

**Controls:**

- account creation
- login success/failure
- login blocking
- password changes
- deposits/withdrawals
- logout

**Relevant reference:** OWASP A09 Security Logging & Alerting Failures.

**Limit:** The demo logs events but does not provide centralized monitoring, tamper-resistant logs, alerting, SIEM integration, or incident response.

### 7. Fail-closed database loading

**Threat:** silently accepting corrupted or malformed account data.

**Controls:**

- versioned database header
- exact field-count checks
- hex decoding checks
- salt/hash length checks
- username/ID validation
- duplicate username detection
- program exits rather than overwriting an untrusted database it could not parse

**Relevant references:** OWASP A08 Software or Data Integrity Failures (partial), A10 Mishandling of Exceptional Conditions.

## Important limitations

This demo does **not** currently provide:

- database encryption at rest
- cryptographic integrity/MAC protection for the database
- multi-factor authentication
- password reset/recovery
- production-grade authorization/RBAC
- TLS/network transport because the program is local-only
- secure password terminal masking
- tamper-resistant audit logs
- centralized monitoring/alerting
- SQL injection defenses because no SQL database is used
- XSS/CSRF/SSRF defenses because no web interface exists

These limits are intentionally documented instead of being hidden behind broad claims such as "prevents the OWASP Top 10."

## Why this mapping matters

The security value of the project comes from being able to explain **which weakness a control addresses, how the control works, and where the control stops**. That style of reasoning is closer to real application-security work than simply listing security buzzwords.
