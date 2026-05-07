#pragma once
#include "core/step.h"

class NvidiaSuspendStep : public Step {
  public:
    std::string name() const override { return "nvidia-suspend"; }
    std::string description() const override;
    std::string preview(const SystemInfo& info) const override;
    bool execute(const SystemInfo& info) override;
    bool applicable(const SystemInfo& info) const override;
};
