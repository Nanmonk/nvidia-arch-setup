#include "driver_install.hpp"
#include "core/utils.hpp"
#include <sstream>

std::string DriverInstallStep::description() const {
    return "Install NVIDIA driver, nvidia-utils, and lib32 packages via pacman";
}

std::string DriverInstallStep::preview(const SystemInfo& info) const {
    const auto& gpu = *info.nvidia_gpu;
    std::string pkgs = gpu.driver_package + " nvidia-utils";
    if (!gpu.lib32_package.empty()) pkgs += " " + gpu.lib32_package;
    if (gpu.driver_package.find("dkms") != std::string::npos) {
        switch (info.kernel) {
            case KernelType::LinuxLts:      pkgs += " linux-lts-headers"; break;
            case KernelType::LinuxZen:      pkgs += " linux-zen-headers"; break;
            case KernelType::LinuxHardened: pkgs += " linux-hardened-headers"; break;
            default:                        pkgs += " linux-headers"; break;
        }
    }
    return "pacman -S --needed " + pkgs;
}

bool DriverInstallStep::applicable(const SystemInfo& info) const {
    return info.nvidia_gpu.has_value();
}

bool DriverInstallStep::execute(const SystemInfo& info) {
    const auto& gpu = *info.nvidia_gpu;

    utils::print_info("GPU detected: " + gpu.name);
    utils::print_info("Driver: " + gpu.driver_package);

    std::string pkgs = gpu.driver_package + " nvidia-utils";
    if (!gpu.lib32_package.empty())
        pkgs += " " + gpu.lib32_package;

    // For dkms drivers, also install linux headers
    if (gpu.driver_package.find("dkms") != std::string::npos) {
        switch (info.kernel) {
            case KernelType::LinuxLts:      pkgs += " linux-lts-headers"; break;
            case KernelType::LinuxZen:      pkgs += " linux-zen-headers"; break;
            case KernelType::LinuxHardened: pkgs += " linux-hardened-headers"; break;
            default:                        pkgs += " linux-headers"; break;
        }
    }

    return utils::exec_interactive("pacman -S --needed " + pkgs);
}
