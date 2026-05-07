#pragma once
#include "core/step.hpp"

class MkinitcpioStep : public Step {
public:
    std::string name() const override { return "Configure mkinitcpio"; }
    std::string description() const override;
    std::string preview(const SystemInfo& info) const override;
    bool applicable(const SystemInfo& info) const override;
    bool execute(const SystemInfo& info) override;
};

class MkinitcpioRebuildStep : public Step {
public:
    std::string name() const override { return "Rebuild initramfs"; }
    std::string description() const override { return "Run mkinitcpio -P to rebuild all presets"; }
    bool execute(const SystemInfo& info) override;
};
