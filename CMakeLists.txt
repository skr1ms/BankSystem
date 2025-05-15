cmake_minimum_required(VERSION 3.20)

# Подключение vcpkg toolchain — должно быть до project()
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(BankSystem)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)

find_package(libpqxx CONFIG REQUIRED)

set(SOURCES src/main.cpp src/BankSystem.cpp src/DatabaseManager.cpp)

add_executable(BankSystem ${SOURCES})

target_include_directories(BankSystem PRIVATE include)

target_link_libraries(BankSystem PRIVATE OpenSSL::SSL OpenSSL::Crypto libpqxx::pqxx)
