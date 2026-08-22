#include "command_common.hpp"

void CommandDeviceInfo::Invoke(Request& req) {
    ZCBOR_STATE_E(txState, 2, req.shell.TxBuffer, sizeof(req.shell.TxBuffer), 1);
    zcbor_map_start_encode(txState, 4);
    zcbor_tstr_put_arr(txState, "name");
    zcbor_tstr_put_arr(txState, "PocketTrustee Demo");
    zcbor_tstr_put_arr(txState, "version");
    zcbor_tstr_put_arr(txState, "2.0");
    zcbor_map_end_encode(txState, 4);
    req.shell.Port.Write({req.shell.TxBuffer, static_cast<size_t>(txState->payload - req.shell.TxBuffer)});
}
