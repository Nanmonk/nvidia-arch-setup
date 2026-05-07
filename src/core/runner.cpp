#include "runner.hpp"
#include "utils.hpp"
#include <iostream>

void Runner::add(std::unique_ptr<Step> step) {
    steps_.push_back(std::move(step));
}

bool Runner::ask_confirm(const Step& step) const {
    std::cout << "\n\033[1m[" << step.name() << "]\033[0m\n";
    std::cout << "  " << step.description() << "\n";
    std::cout << "  Execute? [Y/n] ";
    std::string input;
    std::getline(std::cin, input);
    return input.empty() || input == "y" || input == "Y";
}

void Runner::dry_run_step(const Step& step, const SystemInfo& info) const {
    std::cout << "\n\033[1;33m[DRY] " << step.name() << "\033[0m\n";
    std::cout << "  " << step.preview(info) << "\n";
}

void Runner::run(const SystemInfo& info) {
    int total = 0, done = 0, skipped = 0, failed = 0;

    for (auto& step : steps_)
        if (step->applicable(info)) ++total;

    if (dry_run_) {
        std::cout << "\n\033[1;33m[DRY RUN] No changes will be made.\033[0m "
                  << "(" << total << " steps applicable)\n";
        for (auto& step : steps_) {
            if (step->applicable(info))
                dry_run_step(*step, info);
        }
        std::cout << "\n";
        return;
    }

    std::cout << "\n\033[1mTotal steps: " << total << "\033[0m\n";

    for (auto& step : steps_) {
        if (!step->applicable(info)) continue;

        bool should_run = default_mode_ || ask_confirm(*step);
        if (!should_run) {
            utils::print_warn("Skipped: " + step->name());
            ++skipped;
            continue;
        }

        if (!default_mode_)
            std::cout << "\n";

        utils::print_info("Running: " + step->name());
        bool ok = step->execute(info);

        if (ok) {
            utils::print_ok("Done: " + step->name());
            ++done;
        } else {
            utils::print_err("Failed: " + step->name());
            ++failed;
            if (!default_mode_) {
                std::cout << "  Continue anyway? [Y/n] ";
                std::string input;
                std::getline(std::cin, input);
                if (input == "n" || input == "N") break;
            }
        }
    }

    std::cout << "\n\033[1mSummary:\033[0m "
              << done    << " done, "
              << skipped << " skipped, "
              << failed  << " failed"
              << " (of " << total << " total)\n";
}
