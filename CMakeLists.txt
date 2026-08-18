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

target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    cherryusb
    littlefs
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -u _scanf_float)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        PKT_YES_DEBUG
        DEBUG_SHOW_INFO
        DEBUG_SHOW_WARN
        DEBUG_SHOW_ERROR
    )
endif ()
