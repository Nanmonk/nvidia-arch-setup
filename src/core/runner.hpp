#pragma once
#include "step.hpp"
#include "system_info.hpp"
#include <memory>
#include <vector>

class Runner {
public:
    explicit Runner(bool default_mode, bool dry_run = false)
        : default_mode_(default_mode), dry_run_(dry_run) {}

    void add(std::unique_ptr<Step> step);
    void run(const SystemInfo& info);

private:
    bool ask_confirm(const Step& step) const;
    void dry_run_step(const Step& step, const SystemInfo& info) const;

    std::vector<std::unique_ptr<Step>> steps_;
    bool default_mode_;
    bool dry_run_;
};
