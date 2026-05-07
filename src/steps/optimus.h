#pragma once
#include "core/step.h"

class OptimusStep : public Step {
  public:
    std::string name() const override { return "Configure Optimus (PRIME)"; }
    std::string description() const override;
    bool applicable(const SystemInfo& info) const override;
    bool execute(const SystemInfo& info) override;
};
