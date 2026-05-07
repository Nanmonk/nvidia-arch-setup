#pragma once
#include "core/step.hpp"

class PacmanHookStep : public Step {
public:
    std::string name() const override { return "Install Pacman Hook"; }
    std::string description() const override;
    bool applicable(const SystemInfo& info) const override;
    bool execute(const SystemInfo& info) override;
};
