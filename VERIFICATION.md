# Verification

The finished project was compiled and tested in the working environment with:

- GCC/G++ 14.2
- OpenSSL 3.5.7
- CMake

Verification commands:

```bash
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

Result at packaging time:

```text
100% tests passed, 0 tests failed out of 1
```

A scripted smoke test also verified:

- account creation
- database persistence
- successful login
- balance display
- deposit
- masked profile display
- logout
- audit-log creation

Generated test database and audit-log files were removed before packaging.
