cmake_minimum_required(VERSION 3.22)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_PROJECT_NAME PocketTrustee)

project(${CMAKE_PROJECT_NAME})

enable_language(C CXX ASM)

add_executable(${CMAKE_PROJECT_NAME})

add_subdirectory(cmake/stm32cubemx)
add_subdirectory(src)
add_subdirectory(cmake/littlefs)
add_subdirectory(cmake/cherryusb)
add_subdirectory(cmake/zcbor)
add_subdirectory(cmake/mbedtls)

target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    cherryusb
    littlefs
    zcbor
    mbedtls
)

file(GLOB FREERTOS_PLUS_POSIX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos-posix/FreeRTOS-Plus-POSIX/source/*.c)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos-posix/include
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos-posix/include/private
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos-posix/FreeRTOS-Plus-POSIX/include
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos-posix/FreeRTOS-Plus-POSIX/include/portable
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos-posix/FreeRTOS-Plus-POSIX/include/portable/pc/windows)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${FREERTOS_PLUS_POSIX_SOURCES})

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    PKT_DEBUG_LEVEL=PKT_DEBUG_LEVEL_INFO
    PKT_ENABLE_ASSERT
)

