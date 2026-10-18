# Verification

Current verification is designed to be repeatable from the repository itself.

## Local verification

Requirements:

- C++17 compiler
- CMake 3.16+
- OpenSSL development libraries

Commands:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

The current test executable uses explicit runtime checks rather than `assert()`, so the checks are not disabled by `NDEBUG`.

Coverage includes:

- username/password/identifier validation
- money parsing
- PBKDF2 derivation and constant-time comparison
- password changes
- deposits and withdrawals
- signed 64-bit deposit overflow rejection
- database persistence and reload
- duplicate-username rejection
- temporary authentication lockout
- rejection of malformed numeric database fields

## GitHub Actions

`.github/workflows/ci.yml` runs the configure, build, and CTest sequence on pull requests and pushes to `main`.

This keeps the verification status tied to the current commit instead of preserving a stale one-time packaging result.

## Manual smoke testing

A useful manual smoke test should still cover the interactive path:

- account creation
- database persistence
- successful and failed login
- balance display
- deposit and withdrawal
- password change
- masked profile display
- logout
- audit-log creation

The automated tests do not claim to replace full end-to-end or production security testing.
