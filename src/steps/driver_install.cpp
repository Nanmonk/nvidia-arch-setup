#include "driver_install.hpp"
#include "core/utils.hpp"

static std::string kernel_headers(KernelType k) {
    switch (k) {
        case KernelType::LinuxLts:      return "linux-lts-headers";
        case KernelType::LinuxZen:      return "linux-zen-headers";
        case KernelType::LinuxHardened: return "linux-hardened-headers";
        default:                        return "linux-headers";
    }
}

static std::string aur_helper_cmd(AurHelper h) {
    switch (h) {
        case AurHelper::Paru: return "paru";
        case AurHelper::Yay:  return "yay";
        default:              return "";
    }
}

std::string DriverInstallStep::description() const {
    return "Install NVIDIA driver, nvidia-utils, and lib32 packages";
}

std::string DriverInstallStep::preview(const SystemInfo& info) const {
    const auto& gpu = *info.nvidia_gpu;

    std::string official = "nvidia-utils";
    std::string aur      = gpu.driver_package;

    if (!gpu.lib32_package.empty()) {
        if (gpu.driver_is_aur) aur      += " " + gpu.lib32_package;
        else                   official += " " + gpu.lib32_package;
    }
    if (gpu.driver_is_aur)
        aur += " " + kernel_headers(info.kernel);
    else
        official += " " + gpu.driver_package;

    std::string result;
    result += "pacman -S --needed " + official + "\n";
    if (gpu.driver_is_aur) {
        std::string helper = aur_helper_cmd(info.aur_helper);
        if (helper.empty())
            result += "  (AUR) " + aur + "  — needs paru or yay";
        else
            result += helper + " -S --needed " + aur;
    }
    return result;
}

bool DriverInstallStep::applicable(const SystemInfo& info) const {
    return info.nvidia_gpu.has_value();
}

bool DriverInstallStep::execute(const SystemInfo& info) {
    const auto& gpu = *info.nvidia_gpu;

    utils::print_info("GPU: " + gpu.name);
    utils::print_info("Driver: " + gpu.driver_package +
                      (gpu.driver_is_aur ? " (AUR)" : " (official)"));

    // Split into official repo packages and AUR packages
    std::string official_pkgs = "nvidia-utils";
    std::string aur_pkgs      = gpu.driver_package;

    if (!gpu.lib32_package.empty()) {
        if (gpu.driver_is_aur) aur_pkgs      += " " + gpu.lib32_package;
        else                   official_pkgs += " " + gpu.lib32_package;
    }

    if (gpu.driver_is_aur) {
        aur_pkgs += " " + kernel_headers(info.kernel);
    } else {
        official_pkgs += " " + gpu.driver_package;
    }

    // Install official packages first
    utils::print_info("Installing from official repos: " + official_pkgs);
    if (!utils::exec_interactive("pacman -S --needed " + official_pkgs)) {
        utils::print_err("pacman install failed");
        return false;
    }

    if (!gpu.driver_is_aur) return true;

    // AUR packages
    std::string helper = aur_helper_cmd(info.aur_helper);
    if (helper.empty()) {
        utils::print_warn("No AUR helper (paru/yay) found.");
        utils::print_warn("Install manually from AUR: " + aur_pkgs);
        utils::print_info("Or install paru: https://aur.archlinux.org/packages/paru");
        return false;
    }

    utils::print_info("Installing from AUR with " + helper + ": " + aur_pkgs);
    return utils::exec_interactive(helper + " -S --needed " + aur_pkgs);
}
