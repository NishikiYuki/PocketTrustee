#include "command_common.hpp"

#include "app_ptt_utility.hpp"

#include <cstring>

namespace {
    constexpr size_t kMaxPathLength = LFS_NAME_MAX;
    constexpr size_t kChunkSize = 128;
    constexpr uint32_t kResponseTimeout = 1000;

    using Path = char[kMaxPathLength + 1];

    bool WriteResponse(ShellSocket& shell, size_t size) { return shell.Port.Write({shell.TxBuffer, size}) == size; }

    bool SendText(ShellSocket& shell, std::string_view value) {
        ZCBOR_STATE_E(state, 1, shell.TxBuffer, sizeof(shell.TxBuffer), 1);
        if (!zcbor_tstr_encode_ptr(state, value.data(), value.size())) {
            return false;
        }
        return WriteResponse(shell, static_cast<size_t>(state->payload - shell.TxBuffer));
    }

    bool SendError(ShellSocket& shell, int error) {
        ZCBOR_STATE_E(state, 1, shell.TxBuffer, sizeof(shell.TxBuffer), 2);
        if (!zcbor_list_start_encode(state, 2) || !zcbor_tstr_encode_ptr(state, "error", sizeof("error") - 1) ||
            !zcbor_int32_put(state, error) || !zcbor_list_end_encode(state, 2)) {
            return false;
        }
        return WriteResponse(shell, static_cast<size_t>(state->payload - shell.TxBuffer));
    }

    bool SendSuccess(ShellSocket& shell) { return SendText(shell, "success"); }

    bool SendReady(ShellSocket& shell) { return SendText(shell, "ready"); }

    bool SendDone(ShellSocket& shell) { return SendText(shell, "done"); }

    bool DecodePath(zcbor_state_t* state, Path& path) {
        zcbor_string value{};
        if (!zcbor_tstr_decode(state, &value) || value.len == 0 || value.len > kMaxPathLength) {
            return false;
        }
        if (std::memchr(value.value, '\0', value.len) != nullptr) {
            return false;
        }
        std::memcpy(path, value.value, value.len);
        path[value.len] = '\0';
        return true;
    }

    bool EndRequest(zcbor_state_t* state) {
        return zcbor_list_end_decode(state) && state->payload == state->payload_end;
    }

    bool DecodeAck(ShellSocket& shell, uint16_t size) {
        ZCBOR_STATE_D(state, 1, shell.RxBuffer, size, 1, 0);
        return zcbor_tstr_expect_lit(state, "ok") && state->payload == state->payload_end;
    }

    bool ReceiveAck(ShellSocket& shell) {
        shell.WaitForResponse(kResponseTimeout);
        const uint16_t size = shell.Port.Read(shell.RxBuffer, sizeof(shell.RxBuffer));
        return size != 0 && DecodeAck(shell, size);
    }

    bool DecodeWriteChunk(ShellSocket& shell, uint16_t size, bool& more, zcbor_string& data) {
        ZCBOR_STATE_D(state, 1, shell.RxBuffer, size, 1, 0);
        if (!zcbor_list_start_decode(state) || !zcbor_bool_decode(state, &more) || !zcbor_bstr_decode(state, &data) ||
            data.len > kChunkSize || !zcbor_list_end_decode(state) || state->payload != state->payload_end) {
            return false;
        }
        return true;
    }

    bool SendChunk(ShellSocket& shell, bool more, const uint8_t* data, size_t size) {
        ZCBOR_STATE_E(state, 1, shell.TxBuffer, sizeof(shell.TxBuffer), 2);
        if (!zcbor_list_start_encode(state, 2) || !zcbor_bool_put(state, more) ||
            !zcbor_bstr_encode_ptr(state, reinterpret_cast<const char*>(data), size) ||
            !zcbor_list_end_encode(state, 2)) {
            return false;
        }
        return WriteResponse(shell, static_cast<size_t>(state->payload - shell.TxBuffer));
    }

    bool DecodeSinglePathRequest(ICommand::Request& req, Path& path) {
        return DecodePath(req.state, path) && EndRequest(req.state);
    }

    void SendFilesystemError(ShellSocket& shell, int error) { (void)SendError(shell, error); }
} // namespace

void CommandLs::Invoke(Request& req) {
    Path path{};
    if (!DecodeSinglePathRequest(req, path)) {
        SendFilesystemError(req.shell, LFS_ERR_INVAL);
        return;
    }

    PocketTrustee::LfsDirectoryGuard directory{&req.shell.App.lfs};
    int error = directory.open(path);
    if (error != LFS_ERR_OK) {
        SendFilesystemError(req.shell, error);
        return;
    }

    lfs_info info{};
    while ((error = lfs_dir_read(directory.lfs, &directory.d, &info)) > 0) {
        const char* type = nullptr;
        if (info.type == LFS_TYPE_REG) {
            type = "file";
        }
        else if (info.type == LFS_TYPE_DIR) {
            type = "dir";
        }
        else {
            SendFilesystemError(req.shell, LFS_ERR_IO);
            return;
        }

        ZCBOR_STATE_E(state, 1, req.shell.TxBuffer, sizeof(req.shell.TxBuffer), 3);
        if (!zcbor_list_start_encode(state, 3) || !zcbor_tstr_encode_ptr(state, info.name, std::strlen(info.name)) ||
            !zcbor_tstr_encode_ptr(state, type, std::strlen(type)) || !zcbor_uint32_put(state, info.size) ||
            !zcbor_list_end_encode(state, 3) ||
            !WriteResponse(req.shell, static_cast<size_t>(state->payload - req.shell.TxBuffer)) ||
            !ReceiveAck(req.shell)) {
            SendFilesystemError(req.shell, LFS_ERR_IO);
            return;
        }
    }
    if (error != 0) {
        SendFilesystemError(req.shell, error);
        return;
    }
    (void)SendDone(req.shell);
}

void CommandMkdir::Invoke(Request& req) {
    Path path{};
    if (!DecodeSinglePathRequest(req, path)) {
        SendFilesystemError(req.shell, LFS_ERR_INVAL);
        return;
    }
    const int error = lfs_mkdir(&req.shell.App.lfs, path);
    if (error == LFS_ERR_OK) {
        (void)SendSuccess(req.shell);
    }
    else {
        SendFilesystemError(req.shell, error);
    }
}

void CommandRm::Invoke(Request& req) {
    Path path{};
    if (!DecodeSinglePathRequest(req, path)) {
        SendFilesystemError(req.shell, LFS_ERR_INVAL);
        return;
    }
    const int error = lfs_remove(&req.shell.App.lfs, path);
    if (error == LFS_ERR_OK) {
        (void)SendSuccess(req.shell);
    }
    else {
        SendFilesystemError(req.shell, error);
    }
}

void CommandMv::Invoke(Request& req) {
    Path origin{};
    Path destination{};
    if (!DecodePath(req.state, origin) || !DecodePath(req.state, destination) || !EndRequest(req.state)) {
        SendFilesystemError(req.shell, LFS_ERR_INVAL);
        return;
    }
    const int error = lfs_rename(&req.shell.App.lfs, origin, destination);
    if (error == LFS_ERR_OK) {
        (void)SendSuccess(req.shell);
    }
    else {
        SendFilesystemError(req.shell, error);
    }
}

void CommandWrite::Invoke(Request& req) {
    Path path{};
    if (!DecodeSinglePathRequest(req, path)) {
        SendFilesystemError(req.shell, LFS_ERR_INVAL);
        return;
    }

    PocketTrustee::LfsFileGuard file{&req.shell.App.lfs};
    int error = file.open(path, LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC);
    if (error != LFS_ERR_OK) {
        SendFilesystemError(req.shell, error);
        return;
    }
    if (!SendReady(req.shell)) {
        return;
    }

    for (;;) {
        req.shell.WaitForResponse(kResponseTimeout);
        const uint16_t size = req.shell.Port.Read(req.shell.RxBuffer, sizeof(req.shell.RxBuffer));
        bool more = false;
        zcbor_string data{};
        if (size == 0 || !DecodeWriteChunk(req.shell, size, more, data)) {
            SendFilesystemError(req.shell, LFS_ERR_INVAL);
            return;
        }
        const lfs_ssize_t written = lfs_file_write(file.lfs, &file.f, data.value, data.len);
        if (written != static_cast<lfs_ssize_t>(data.len)) {
            SendFilesystemError(req.shell, written < 0 ? static_cast<int>(written) : LFS_ERR_IO);
            return;
        }
        if (!SendText(req.shell, "ok")) {
            return;
        }
        if (!more) {
            break;
        }
    }

    error = lfs_file_sync(file.lfs, &file.f);
    if (error != LFS_ERR_OK) {
        SendFilesystemError(req.shell, error);
        return;
    }
    (void)SendDone(req.shell);
}

void CommandRead::Invoke(Request& req) {
    Path path{};
    if (!DecodeSinglePathRequest(req, path)) {
        SendFilesystemError(req.shell, LFS_ERR_INVAL);
        return;
    }

    PocketTrustee::LfsFileGuard file{&req.shell.App.lfs};
    int error = file.open(path, LFS_O_RDONLY);
    if (error != LFS_ERR_OK) {
        SendFilesystemError(req.shell, error);
        return;
    }
    const lfs_soff_t file_size = lfs_file_size(file.lfs, &file.f);
    if (file_size < 0) {
        SendFilesystemError(req.shell, static_cast<int>(file_size));
        return;
    }
    if (!SendReady(req.shell) || !ReceiveAck(req.shell)) {
        SendFilesystemError(req.shell, LFS_ERR_IO);
        return;
    }

    lfs_size_t remaining = static_cast<lfs_size_t>(file_size);
    do {
        const lfs_size_t requested = remaining > kChunkSize ? kChunkSize : remaining;
        const lfs_ssize_t read = lfs_file_read(file.lfs, &file.f, req.shell.RxBuffer, requested);
        if (read != static_cast<lfs_ssize_t>(requested)) {
            SendFilesystemError(req.shell, read < 0 ? static_cast<int>(read) : LFS_ERR_IO);
            return;
        }
        remaining -= requested;
        if (!SendChunk(req.shell, remaining != 0, req.shell.RxBuffer, requested) || !ReceiveAck(req.shell)) {
            SendFilesystemError(req.shell, LFS_ERR_IO);
            return;
        }
    }
    while (remaining != 0);

    (void)SendDone(req.shell);
}
