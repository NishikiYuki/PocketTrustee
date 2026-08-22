#include "command_common.hpp"

void CommandDeviceInfo::Invoke(Request& req) {
    std::string_view confirm = "confirm";
    req.shell.Port.Write({reinterpret_cast<const uint8_t*>(confirm.data()), confirm.size()});
    req.shell.WaitForResponse(1000);
    if (req.shell.Port.Read(req.shell.RxBuffer, 32) == 0) {
        return;
    }
    ZCBOR_STATE_E(txState, 2, req.shell.TxBuffer, sizeof(req.shell.TxBuffer), 1);
    zcbor_list_start_encode(txState, 2);
    zcbor_tstr_put_arr(txState, "PocketTrustee Demo");
    zcbor_tstr_put_arr(txState, "2.0");
    zcbor_list_end_encode(txState, 2);
    req.shell.Port.Write({req.shell.TxBuffer, static_cast<size_t>(txState->payload - req.shell.TxBuffer)});
}
