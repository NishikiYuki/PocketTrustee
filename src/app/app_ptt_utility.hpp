#ifndef POCKETTRUSTEE_APP_PTT_UTILITY_HPP
#define POCKETTRUSTEE_APP_PTT_UTILITY_HPP

#include "common_log.h"
#include <cstdint>
#include <string_view>
#include <memory>
#include "lfs.h"

namespace PocketTrustee {
    enum class TimeZoneOffset : int32_t {
        None = 0,
        Shanghai = (+8 * 3600)
    };

    struct DateTime {
        uint16_t year{};
        uint8_t month{};
        uint8_t day{};
        uint8_t hour{};
        uint8_t minute{};
        uint8_t second{};
        TimeZoneOffset zone = TimeZoneOffset::None;
        uint64_t ToUnixTime() const;
        static DateTime FromUnixTime(uint64_t unix_time, TimeZoneOffset offset = TimeZoneOffset::None);
    };

    class LfsFileGuard final {
    public:
        explicit LfsFileGuard(lfs_t * fs) : lfs(fs) {}

        int open(const char *path, int flags) {
            int err = lfs_file_open(lfs, &f, path, flags);
            if (err == LFS_ERR_OK) {
                opened_ = true;
            }
            return err;
        }

        ~LfsFileGuard() {
            if (opened_) {
                lfs_file_close(lfs, &f);
            }
        }

        lfs_t* lfs;
        lfs_file_t f{};
    private:
        bool opened_ = false;
    };

    class LfsDirectoryGuard final {
    public:
        explicit LfsDirectoryGuard(lfs_t * fs) : lfs(fs) {}

        int open(const char *path) {
            int err = lfs_dir_open(lfs, &d, path);
            if (err == LFS_ERR_OK) {
                opened_ = true;
            }
            return err;
        }
        
        ~LfsDirectoryGuard() {
            if (opened_) {
                lfs_dir_close(lfs, &d);
            }
        }

        lfs_t* lfs;
        lfs_dir_t d{};
    private:
        bool opened_ = false;
    };
}

#endif //POCKETTRUSTEE_APP_PTT_UTILITY_HPP
