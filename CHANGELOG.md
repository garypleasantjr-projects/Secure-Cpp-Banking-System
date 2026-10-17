# Changelog

## Version 2.0 - Finished secure-coding portfolio version

- completed account creation/login/session flow
- completed database load/save behavior
- preserved password salts when loading accounts
- increased PBKDF2-HMAC-SHA256 work factor for new credentials
- added constant-time hash comparison
- added login throttling and temporary lockout
- added generic authentication failures
- added audit logging
- changed balances from floating-point dollars to integer cents
- changed full identification number storage to fictional/demo last-four digits only
- added password-change flow with fresh salts
- added fail-closed database validation
- added CMake build system
- added automated core tests
- added OWASP/CWE security mapping
- added learning guide
