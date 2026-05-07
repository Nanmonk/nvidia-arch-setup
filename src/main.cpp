#include <iostream>
#include <memory>
#include <unistd.h>
#include "core/system_info.hpp"
#include "core/runner.hpp"
#include "core/utils.hpp"
#include "steps/driver_install.hpp"
#include "steps/mkinitcpio.hpp"
#include "steps/kernel_params.hpp"
#include "steps/pacman_hook.hpp"
#include "steps/optimus.hpp"

static void print_banner() {
    std::cout <<
        "\n\033[1;34m╔══════════════════════════════════════╗\033[0m\n"
        "\033[1;34m║    nvidia-arch-setup  v1.0           ║\033[0m\n"
        "\033[1;34m║    NVIDIA auto-configurator for Arch ║\033[0m\n"
        "\033[1;34m╚══════════════════════════════════════╝\033[0m\n\n";
}

static void print_sysinfo(const SystemInfo& info) {
    std::cout << "\033[1mSystem Detection:\033[0m\n";

    if (info.nvidia_gpu) {
        const auto& g = *info.nvidia_gpu;
        utils::print_ok("NVIDIA GPU : " + g.name);
        utils::print_info("  Driver   : " + g.driver_package);
        if (!g.lib32_package.empty())
            utils::print_info("  lib32    : " + g.lib32_package);
    } else {
        utils::print_err("No NVIDIA GPU detected");
    }

    if (info.is_optimus) {
        std::string igpu = info.igpu_vendor == IgpuVendor::Intel ? "Intel" : "AMD";
        utils::print_ok("Optimus    : " + igpu + " iGPU + NVIDIA dGPU");
    }

    std::string kname;
    switch (info.kernel) {
        case KernelType::LinuxLts:      kname = "linux-lts"; break;
        case KernelType::LinuxZen:      kname = "linux-zen"; break;
        case KernelType::LinuxHardened: kname = "linux-hardened"; break;
        case KernelType::Custom:        kname = "custom"; break;
        default:                        kname = "linux"; break;
    }
    utils::print_info("Kernel     : " + kname + " (" + info.kernel_ver + ")");

    std::string bl;
    switch (info.bootloader) {
        case Bootloader::Grub:        bl = "GRUB"; break;
        case Bootloader::SystemdBoot: bl = "systemd-boot"; break;
        case Bootloader::Refind:      bl = "rEFInd"; break;
        default:                      bl = "unknown"; break;
    }
    utils::print_info("Bootloader : " + bl);

    std::string sess;
    switch (info.session) {
        case SessionType::Wayland: sess = "Wayland"; break;
        case SessionType::X11:     sess = "X11"; break;
        default:                   sess = "unknown"; break;
    }
    utils::print_info("Session    : " + sess);
    std::cout << "\n";
}

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog << " [--default] [--dry-run]\n"
              << "  --default   Run all steps without interactive confirmation\n"
              << "  --dry-run   Show what would be done without making any changes\n";
}

int main(int argc, char* argv[]) {
    bool default_mode = false;
    bool dry_run      = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--default") {
            default_mode = true;
        } else if (arg == "--dry-run") {
            dry_run = true;
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    if (!dry_run && geteuid() != 0) {
        std::cerr << "\033[31m[✗]\033[0m Must be run as root (use sudo)\n";
        return 1;
    }

    print_banner();

    utils::print_info("Detecting system...");
    auto info = SystemInfo::detect();
    print_sysinfo(info);

    if (!info.nvidia_gpu) {
        utils::print_err("No NVIDIA GPU found, nothing to do.");
        return 1;
    }

    if (default_mode)
        utils::print_warn("Running in DEFAULT mode (no confirmation prompts)\n");
    else
        utils::print_info("Running in INTERACTIVE mode (confirm each step)\n");

    Runner runner(default_mode, dry_run);
    runner.add(std::make_unique<DriverInstallStep>());
    runner.add(std::make_unique<MkinitcpioStep>());
    runner.add(std::make_unique<KernelParamsStep>());
    runner.add(std::make_unique<PacmanHookStep>());
    runner.add(std::make_unique<OptimusStep>());
    runner.add(std::make_unique<MkinitcpioRebuildStep>());
    runner.run(info);

    utils::print_info("Reboot to apply changes.");
    return 0;
}
