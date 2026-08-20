cmake_minimum_required(VERSION 3.22)

add_library(cherryusb OBJECT)

target_link_libraries(cherryusb stm32cubemx)

target_include_directories(cherryusb PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/cherryusb/class/cdc/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/cherryusb/class/hid/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/cherryusb/common/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/cherryusb/core/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/cherryusb/port/fsdev/
)

target_sources(cherryusb PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/cherryusb/class/cdc/usbd_cdc_acm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/cherryusb/class/hid/usbd_hid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/cherryusb/core/usbd_core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/cherryusb/port/fsdev/usb_glue_st.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Lib/cherryusb/port/fsdev/usb_dc_fsdev.c
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(USB_COMPILE_DEFINES
        CONFIG_USB_DBG_LEVEL=USB_DBG_INFO
    )
else ()
    set(USB_COMPILE_DEFINES
        CONFIG_USB_DBG_LEVEL=USB_DBG_ERROR
        CONFIG_USB_ASSERT_DISABLE
    )
endif ()

target_compile_definitions(cherryusb PRIVATE ${USB_COMPILE_DEFINES})