#include "optimus.hpp"
#include "core/utils.hpp"

static const std::string XORG_CONF =
    "/etc/X11/xorg.conf.d/10-nvidia-drm-outputclass.conf";

static const std::string XORG_CONTENT =
    "Section \"OutputClass\"\n"
    "    Identifier \"integrated\"\n"
    "    MatchDriver \"i915\"\n"
    "    Driver \"modesetting\"\n"
    "EndSection\n"
    "\n"
    "Section \"OutputClass\"\n"
    "    Identifier \"nvidia\"\n"
    "    MatchDriver \"nvidia-drm\"\n"
    "    Driver \"nvidia\"\n"
    "    Option \"AllowEmptyInitialConfiguration\"\n"
    "    Option \"PrimaryGPU\" \"yes\"\n"
    "    ModulePath \"/usr/lib/nvidia/xorg\"\n"
    "    ModulePath \"/usr/lib/xorg/modules\"\n"
    "EndSection\n";

std::string OptimusStep::description() const {
    return "Configure PRIME render offload for Optimus (hybrid Intel/AMD + NVIDIA)";
}

bool OptimusStep::applicable(const SystemInfo& info) const {
    return info.is_optimus;
}

bool OptimusStep::execute(const SystemInfo& info) {
    utils::print_info("Optimus detected: " +
        std::string(info.igpu_vendor == IgpuVendor::Intel ? "Intel" : "AMD") +
        " iGPU + NVIDIA dGPU");

    // Install required packages
    utils::print_info("Installing xorg-xrandr...");
    if (!utils::exec_interactive("pacman -S --needed xorg-xrandr"))
        utils::print_warn("xorg-xrandr install failed, continuing...");

    // Write Xorg outputclass config for X11/XWayland
    utils::exec_interactive("mkdir -p /etc/X11/xorg.conf.d");
    if (!utils::write_file(XORG_CONF, XORG_CONTENT)) {
        utils::print_err("Cannot write " + XORG_CONF);
        return false;
    }
    utils::print_ok("Xorg outputclass config written");

    // SDDM setup script for Xsetup
    if (utils::file_exists("/usr/share/sddm/scripts/Xsetup")) {
        auto xsetup = utils::read_file("/usr/share/sddm/scripts/Xsetup");
        if (xsetup && xsetup->find("setprovideroutputsource") == std::string::npos) {
            std::string append =
                "\nxrandr --setprovideroutputsource modesetting NVIDIA-0\n"
                "xrandr --auto\n";
            utils::write_file("/usr/share/sddm/scripts/Xsetup", *xsetup + append);
            utils::print_ok("SDDM Xsetup updated for PRIME");
        }
    }

    // Wayland: DRM modeset is mandatory for PRIME on Wayland (already handled by KernelParamsStep)
    if (info.session == SessionType::Wayland) {
        utils::print_info(
            "On Wayland, use 'prime-run <app>' or set __NV_PRIME_RENDER_OFFLOAD=1 "
            "__GLX_VENDOR_LIBRARY_NAME=nvidia for per-app GPU selection");
    }

    // Create prime-run wrapper if not present
    if (!utils::file_exists("/usr/local/bin/prime-run")) {
        const std::string prime_run =
            "#!/bin/bash\n"
            "__NV_PRIME_RENDER_OFFLOAD=1 "
            "__NV_PRIME_RENDER_OFFLOAD_PROVIDER=NVIDIA-G0 "
            "__GLX_VENDOR_LIBRARY_NAME=nvidia "
            "__VK_LAYER_NV_optimus=NVIDIA_only "
            "exec \"$@\"\n";
        utils::write_file("/usr/local/bin/prime-run", prime_run);
        utils::exec_interactive("chmod +x /usr/local/bin/prime-run");
        utils::print_ok("prime-run wrapper created at /usr/local/bin/prime-run");
    }

    return true;
}
