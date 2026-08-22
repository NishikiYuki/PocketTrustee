#ifndef POCKETTRUSTEE_COMMAND_COMMON_HPP
#define POCKETTRUSTEE_COMMAND_COMMON_HPP

#include "app_shell.hpp"
#include "zcbor_common.h"
#include "zcbor_decode.h"
#include "zcbor_encode.h"

class ICommand {
public:
    struct Request {
        zcbor_state_t* state;
        ShellSocket& shell;
    };

    virtual ~ICommand() = default;
    virtual std::string_view Name() const noexcept = 0;
    virtual void Invoke(Request& req) = 0;
};

class CommandDeviceInfo final : public ICommand {
public:
    std::string_view Name() const noexcept override { return "info"; }
    void Invoke(Request& req) override;
};

class CommandLs final : public ICommand {
public:
    std::string_view Name() const noexcept override { return "ls"; }
    void Invoke(Request& req) override;
};

class CommandMkdir final : public ICommand {
public:
    std::string_view Name() const noexcept override { return "mkdir"; }
    void Invoke(Request& req) override;
};

class CommandRm final : public ICommand {
public:
    std::string_view Name() const noexcept override { return "rm"; }
    void Invoke(Request& req) override;
};

class CommandMv final : public ICommand {
public:
    std::string_view Name() const noexcept override { return "mv"; }
    void Invoke(Request& req) override;
};

class CommandWrite final : public ICommand {
public:
    std::string_view Name() const noexcept override { return "write"; }
    void Invoke(Request& req) override;
};

class CommandRead final : public ICommand {
public:
    std::string_view Name() const noexcept override { return "read"; }
    void Invoke(Request& req) override;
};

class CommandFormat final : public ICommand {
public:
    std::string_view Name() const noexcept override { return "format"; }
    void Invoke(Request& req) override;
};

#endif // POCKETTRUSTEE_COMMAND_COMMON_HPP
