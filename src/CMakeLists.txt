cmake_minimum_required(VERSION 3.22)

set(SourceIncDir
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/driver
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/gui/view
    ${CMAKE_CURRENT_SOURCE_DIR}/ll
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SOURCE_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/app SOURCE_APP_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/common SOURCE_COMMON_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/driver SOURCE_DRIVER_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gui SOURCE_GUI_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gui/view SOURCE_VIEW_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ll SOURCE_LL_LIST)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${SourceIncDir})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${SOURCE_LIST}
    ${SOURCE_APP_LIST}
    ${SOURCE_COMMON_LIST}
    ${SOURCE_DRIVER_LIST}
    ${SOURCE_LL_LIST}
    ${SOURCE_GUI_LIST}
    ${SOURCE_VIEW_LIST}
)