cmake_minimum_required(VERSION 3.22)

add_library(littlefs OBJECT)

target_include_directories(littlefs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/littlefs
)

target_sources(littlefs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/littlefs/lfs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/littlefs/lfs_util.c
)

target_compile_definitions(littlefs PRIVATE
    LFS_DEFINES=lfs_config.h
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(littlefs PRIVATE
        LFS_NO_DEBUG
        LFS_NO_WARN
        LFS_NO_ERROR
        LFS_NO_ASSERT
    )
endif ()
