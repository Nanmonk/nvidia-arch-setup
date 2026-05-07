#include "pacman_hook.hpp"
#include "core/utils.hpp"

static const std::string HOOK_DIR  = "/etc/pacman.d/hooks";
static const std::string HOOK_PATH = HOOK_DIR + "/nvidia.hook";

std::string PacmanHookStep::description() const {
    return "Install " + HOOK_PATH + " to auto-rebuild initramfs on driver updates";
}

bool PacmanHookStep::applicable(const SystemInfo& info) const {
    return info.nvidia_gpu.has_value();
}

bool PacmanHookStep::execute(const SystemInfo& info) {
    const auto& gpu = *info.nvidia_gpu;

    // Derive utils package: "nvidia-580xx-dkms" -> "nvidia-580xx-utils"
    // For official drivers the utils package is always "nvidia-utils".
    std::string utils_pkg;
    if (gpu.driver_is_aur) {
        utils_pkg = gpu.driver_package;
        auto pos  = utils_pkg.find("-dkms");
        if (pos != std::string::npos)
            utils_pkg.replace(pos, 5, "-utils");
    } else {
        utils_pkg = "nvidia-utils";
    }

    std::string hook =
        "[Trigger]\n"
        "Operation=Install\n"
        "Operation=Upgrade\n"
        "Operation=Remove\n"
        "Type=Package\n"
        "Target=" + gpu.driver_package + "\n"
        "Target=" + utils_pkg + "\n"
        "Target=linux\n"
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
