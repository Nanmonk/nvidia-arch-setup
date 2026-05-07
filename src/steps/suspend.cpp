#include "suspend.hpp"
#include "core/utils.hpp"
#include <vector>
#include <string>

static const std::vector<std::string> SERVICES = {
    "nvidia-suspend.service",
    "nvidia-resume.service",
    "nvidia-hibernate.service",
};

std::string NvidiaSuspendStep::description() const {
    return "Enable nvidia-suspend/resume/hibernate services (prevents black screen on wakeup)";
}

std::string NvidiaSuspendStep::preview(const SystemInfo& /* info */) const {
    std::string result;
    for (const auto& svc : SERVICES)
        result += "systemctl enable " + svc + "\n";
    return result;
}

bool NvidiaSuspendStep::applicable(const SystemInfo& info) const {
    return info.nvidia_gpu.has_value();
}

bool NvidiaSuspendStep::execute(const SystemInfo& /* info */) {
    bool ok = true;
    for (const auto& svc : SERVICES) {
        utils::print_info("Enabling " + svc + "...");
        if (!utils::exec_interactive("systemctl enable " + svc)) {
            utils::print_warn("Failed to enable " + svc +
                              " (may not exist until after reboot with driver loaded)");
            ok = false;
        }
    }
    return ok;
}
