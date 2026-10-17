cmake_minimum_required(VERSION 3.16)
project(GarySecureBankingDemo VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

add_library(bankcore
    src/Account.cpp
    src/AccountManager.cpp
    src/AuditLogger.cpp
    src/Crypto.cpp
    src/InputHandling.cpp
)

target_include_directories(bankcore PUBLIC src)
target_link_libraries(bankcore PUBLIC OpenSSL::Crypto)

if (MSVC)
    target_compile_options(bankcore PRIVATE /W4)
else()
    target_compile_options(bankcore PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(secure_bank src/main.cpp)
target_link_libraries(secure_bank PRIVATE bankcore)

enable_testing()
add_executable(test_core tests/test_core.cpp)
target_link_libraries(test_core PRIVATE bankcore)
add_test(NAME core_tests COMMAND test_core)
