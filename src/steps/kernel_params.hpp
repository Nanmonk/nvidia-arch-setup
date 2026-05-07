#pragma once
#include "core/step.hpp"

class KernelParamsStep : public Step {
public:
    std::string name() const override { return "Set Kernel Parameters"; }
    std::string description() const override;
    bool applicable(const SystemInfo& info) const override;
    bool execute(const SystemInfo& info) override;

private:
    bool apply_grub(const SystemInfo& info);
    bool apply_systemd_boot(const SystemInfo& info);
};
