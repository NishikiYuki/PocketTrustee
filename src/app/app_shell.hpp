#ifndef POCKETTRUSTEE_APP_SHELL_HPP
#define POCKETTRUSTEE_APP_SHELL_HPP

#include "app_ptt.hpp"

class ShellSocket final : PocketTrustee::IService {
    explicit ShellSocket(PocketTrustee::App& app);

public:
    ShellSocket(const ShellSocket&) = delete;
    ShellSocket& operator=(const ShellSocket&) = delete;

    static IService& GetInstance();

    std::string_view Name() const noexcept override { return "shell"; }
    void Start() override;
    void Stop() override;

    PocketTrustee::App& App;
};

#endif //POCKETTRUSTEE_APP_SHELL_HPP
