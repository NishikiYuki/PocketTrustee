#include "app_ptt.hpp"
#include "ll_epaper.hpp"
#include "driver_w25q16.hpp"

using namespace PocketTrustee;

App& App::InitOrGetApp() {
    static guiv2::Gui gui{DeviceEpaper::getInstance()};
    static App app{
        gui,
        ExternalW25Q16::getConfig(),
        DeviceUsb::GetInstance()
    };
    if (lfs_mount(&app.lfs, app.disk) != LFS_ERR_OK &&
        lfs_format(&app.lfs, app.disk) != LFS_ERR_OK &&
        lfs_mount(&app.lfs, app.disk) != LFS_ERR_OK) {
        DEBUG_ERROR("Fail to mount disk");
        Error_Handler();
    }
    return app;
}
