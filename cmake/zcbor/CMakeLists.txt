cmake_minimum_required(VERSION 3.22)

add_library(zcbor OBJECT)

target_include_directories(zcbor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/zcbor/include
)

target_sources(zcbor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/zcbor/src/zcbor_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/zcbor/src/zcbor_decode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/zcbor/src/zcbor_encode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/zcbor/src/zcbor_print.c
)

target_compile_options(zcbor PRIVATE -w -Os -g0)
