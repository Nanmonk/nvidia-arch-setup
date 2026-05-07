#include "pacman_hook.h"

#include "core/utils.h"

static const std::string HOOK_DIR = "/etc/pacman.d/hooks";
static const std::string HOOK_PATH = HOOK_DIR + "/nvidia.hook";

std::string PacmanHookStep::description() const {
    return "Install " + HOOK_PATH + " to auto-rebuild initramfs on driver updates";
}

bool PacmanHookStep::applicable(const SystemInfo& info) const {
    return info.nvidia_gpu.has_value();
}

bool PacmanHookStep::execute(const SystemInfo& info) {
    const auto& gpu = *info.nvidia_gpu;

    std::string hook = "[Trigger]\n"
                       "Operation=Install\n"
                       "Operation=Upgrade\n"
                       "Operation=Remove\n"
                       "Type=Package\n"
                       "Target=" +
                       gpu.driver_package + "\n";
    if (!gpu.utils_package.empty())
        hook += "Target=" + gpu.utils_package + "\n";
    hook += "Target=linux\n"
            "\n"
            "[Action]\n"
            "Description=Updating NVIDIA module in initramfs\n"
            "When=PostTransaction\n"
            "Exec=/usr/bin/mkinitcpio -P\n";

    utils::exec_interactive("mkdir -p " + HOOK_DIR);

    if (!utils::write_file(HOOK_PATH, hook)) {
        utils::print_err("Cannot write " + HOOK_PATH);
        return false;
    }
    utils::print_ok("Hook written to " + HOOK_PATH);
    return true;
}
