#ifndef POCKETTRUSTEE_COMMAND_COMMON_HPP
#define POCKETTRUSTEE_COMMAND_COMMON_HPP

#include "app_shell.hpp"
#include "zcbor_common.h"
#include "zcbor_encode.h"
#include "zcbor_decode.h"

class ICommand {
public:
    struct Request {
        zcbor_state_t * state;
        ShellSocket& shell;
    };

    virtual ~ICommand() = default;
    virtual std::string_view Name() const noexcept = 0;
    virtual void Invoke(Request& req) = 0;
};

class CommandDeviceInfo final : public ICommand {
public:
    std::string_view Name() const noexcept override {return "info";}
    void Invoke(Request& req) override;
};

#endif //POCKETTRUSTEE_COMMAND_COMMON_HPP
