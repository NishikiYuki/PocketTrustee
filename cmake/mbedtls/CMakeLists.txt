cmake_minimum_required(VERSION 3.22)

set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(MBEDTLS_FATAL_WARNINGS OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MBEDTLS_LIBRARY ON CACHE BOOL "" FORCE)
set(USE_SHARED_MBEDTLS_LIBRARY OFF CACHE BOOL "" FORCE)
set(DISABLE_PACKAGE_CONFIG_AND_INSTALL ON CACHE BOOL "" FORCE)
set(INSTALL_MBEDTLS_HEADERS OFF CACHE BOOL "" FORCE)
set(GEN_FILES ON CACHE BOOL "" FORCE)

# Mbed TLS only uses Threads when explicitly requested. This firmware has no
# pthread implementation, and probing it is invalid for the bare-metal target.
set(CMAKE_DISABLE_FIND_PACKAGE_Threads ON)

set(MBEDTLS_USER_CONFIG_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_user_config.h
    CACHE FILEPATH "Mbed TLS user configuration" FORCE
)
set(MBEDTLS_CONFIG_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/mbedtls/configs/config-symmetric-only.h
    CACHE FILEPATH "Mbed TLS base configuration" FORCE
)
set(TF_PSA_CRYPTO_USER_CONFIG_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_user_config.h
    CACHE FILEPATH "TF-PSA-Crypto user configuration" FORCE
)

add_compile_options(-Os -g0 -w)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/mbedtls
    ${CMAKE_CURRENT_BINARY_DIR}/mbedtls
    EXCLUDE_FROM_ALL
)

target_compile_options(builtin PRIVATE -O2 -g0)

target_sources(tfpsacrypto PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_platform.c
)

target_link_libraries(tfpsacrypto stm32cubemx)
