#pragma once
#include "core/step.h"

class DriverInstallStep : public Step {
  public:
    std::string name() const override { return "Install NVIDIA Driver"; }
    std::string description() const override;
    std::string preview(const SystemInfo& info) const override;
    bool applicable(const SystemInfo& info) const override;
    bool execute(const SystemInfo& info) override;
};
