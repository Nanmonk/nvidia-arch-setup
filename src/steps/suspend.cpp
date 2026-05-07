#include "suspend.hpp"
#include "core/utils.hpp"

static const char* ENABLE_CMD =
    "systemctl enable nvidia-suspend.service nvidia-resume.service nvidia-hibernate.service";

std::string NvidiaSuspendStep::description() const {
    return "Enable nvidia-suspend/resume/hibernate services (prevents black screen on wakeup)";
}

std::string NvidiaSuspendStep::preview(const SystemInfo& /* info */) const {
    return std::string(ENABLE_CMD) + "\n";
}

bool NvidiaSuspendStep::applicable(const SystemInfo& info) const {
    return info.nvidia_gpu.has_value();
}

bool NvidiaSuspendStep::execute(const SystemInfo& /* info */) {
    if (!utils::exec_interactive(ENABLE_CMD)) {
        utils::print_warn("Failed to enable suspend services"
                          " (may not exist until after reboot with driver loaded)");
        return false;
    }
    return true;
}
