#include "driver_install.h"

#include "core/utils.h"

#include <cstdlib>
#include <unistd.h>

static std::string kernel_headers(KernelType k) {
    switch (k) {
    case KernelType::LinuxLts:
        return "linux-lts-headers";
    case KernelType::LinuxZen:
        return "linux-zen-headers";
    case KernelType::LinuxHardened:
        return "linux-hardened-headers";
    default:
        return "linux-headers";
    }
}

static std::string aur_helper_cmd(AurHelper h) {
    switch (h) {
    case AurHelper::Paru:
        return "paru";
    case AurHelper::Yay:
        return "yay";
    default:
        return "";
    }
}

std::string DriverInstallStep::description() const {
    return "Install NVIDIA driver, nvidia-utils, and lib32 packages";
}

std::string DriverInstallStep::preview(const SystemInfo& info) const {
    const auto& gpu = *info.nvidia_gpu;
    std::string result;

    if (gpu.driver_is_aur) {
        // Legacy drivers: only headers from pacman; driver+utils from AUR
        result += "pacman -S --needed " + kernel_headers(info.kernel) + "\n";
        std::string aur = gpu.driver_package;
        if (!gpu.lib32_package.empty())
            aur += " " + gpu.lib32_package;
        std::string helper = aur_helper_cmd(info.aur_helper);
        if (helper.empty())
            result += "  (AUR) " + aur + "  — needs paru or yay";
        else
            result += helper + " -S --needed " + aur;
    } else {
        std::string official = gpu.driver_package + " nvidia-utils";
        if (!gpu.lib32_package.empty())
            official += " " + gpu.lib32_package;
        result += "pacman -S --needed " + official;
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
                      (gpu.driver_is_aur ? " (AUR legacy)" : " (official)"));

    if (!gpu.driver_is_aur) {
        // Official driver (Turing+): all packages in official repos
        std::string official_pkgs = gpu.driver_package + " nvidia-utils";
        if (!gpu.lib32_package.empty())
            official_pkgs += " " + gpu.lib32_package;
        utils::print_info("Installing from official repos: " + official_pkgs);
        return utils::exec_interactive("pacman -S --needed " + official_pkgs);
    }

    // Legacy AUR driver (Pascal/Maxwell/Volta/Kepler/Fermi):
    // nvidia-580xx-utils conflicts with nvidia-utils — do NOT install nvidia-utils via pacman.
    // Only install kernel headers (DKMS build dependency) from official repos.
    std::string headers = kernel_headers(info.kernel);
    utils::print_info("Installing kernel headers for DKMS: " + headers);
    if (!utils::exec_interactive("pacman -S --needed " + headers)) {
        utils::print_err("Failed to install kernel headers");
        return false;
    }

    std::string aur_pkgs = gpu.driver_package;
    if (!gpu.lib32_package.empty())
        aur_pkgs += " " + gpu.lib32_package;

    std::string helper = aur_helper_cmd(info.aur_helper);
    if (helper.empty()) {
        utils::print_warn("No AUR helper (paru/yay) found.");
        utils::print_warn("Install manually from AUR: " + aur_pkgs);
        utils::print_warn(
            "Note: remove nvidia-utils first if installed (conflicts with AUR utils)");
        utils::print_info("Install paru: https://aur.archlinux.org/packages/paru");
        return false;
    }

    // paru/yay refuse to run as root; drop privileges back to the invoking user.
    std::string aur_cmd;
    if (geteuid() == 0) {
        const char* sudo_user = std::getenv("SUDO_USER");
        if (!sudo_user) {
            utils::print_err("Running as root without SUDO_USER. Cannot invoke AUR helper.");
            utils::print_warn("Run the tool via sudo from a regular user: sudo nvidia-arch-setup");
            utils::print_warn("Or install manually: " + aur_pkgs);
            return false;
        }
        aur_cmd = "sudo -u " + std::string(sudo_user) + " " + helper + " -S --needed " + aur_pkgs;
    } else {
        aur_cmd = helper + " -S --needed " + aur_pkgs;
    }

    utils::print_info("Installing from AUR with " + helper + ": " + aur_pkgs);
    return utils::exec_interactive(aur_cmd);
}
