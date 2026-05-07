#include "system_info.h"

#include "utils.h"

#include <algorithm>
#include <cctype>

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

NvidiaArch SystemInfo::arch_from_name(const std::string& name) {
    std::string n = to_lower(name);

    // RTX 50xx / Blackwell
    if (n.find("gb1") != std::string::npos || n.find("gb2") != std::string::npos ||
        n.find("rtx 50") != std::string::npos)
        return NvidiaArch::Blackwell;

    // RTX 40xx / Ada Lovelace
    if (n.find("ad1") != std::string::npos || n.find("rtx 40") != std::string::npos)
        return NvidiaArch::AdaLovelace;

    // RTX 30xx / Ampere
    if (n.find("ga1") != std::string::npos || n.find("rtx 30") != std::string::npos ||
        n.find("rtx a") != std::string::npos)
        return NvidiaArch::Ampere;

    // RTX 20xx / GTX 16xx / Turing
    if (n.find("tu1") != std::string::npos || n.find("rtx 20") != std::string::npos ||
        n.find("gtx 16") != std::string::npos)
        return NvidiaArch::Turing;

    // Volta (rare, V100 etc)
    if (n.find("gv1") != std::string::npos || n.find("v100") != std::string::npos ||
        n.find("titan v") != std::string::npos)
        return NvidiaArch::Volta;

    // GTX 10xx / Pascal
    if (n.find("gp1") != std::string::npos || n.find("gtx 10") != std::string::npos ||
        n.find("titan x") != std::string::npos || n.find("titan xp") != std::string::npos)
        return NvidiaArch::Pascal;

    // GTX 9xx / Maxwell
    if (n.find("gm1") != std::string::npos || n.find("gm2") != std::string::npos ||
        n.find("gtx 9") != std::string::npos || n.find("gtx 8") != std::string::npos ||
        n.find("gtx 750") != std::string::npos)
        return NvidiaArch::Maxwell;

    // GTX 6xx / 7xx / Kepler
    if (n.find("gk1") != std::string::npos || n.find("gk2") != std::string::npos ||
        n.find("gtx 7") != std::string::npos || n.find("gtx 6") != std::string::npos)
        return NvidiaArch::Kepler;

    // GTX 4xx / 5xx / Fermi
    if (n.find("gf1") != std::string::npos || n.find("gtx 5") != std::string::npos ||
        n.find("gtx 4") != std::string::npos)
        return NvidiaArch::Fermi;

    // Tesla (GT 200 era)
    if (n.find("gt2") != std::string::npos || n.find("g80") != std::string::npos ||
        n.find("g90") != std::string::npos)
        return NvidiaArch::Tesla;

    return NvidiaArch::Unknown;
}

// nvidia-open (pre-built) only ships for linux and linux-lts.
// Zen/Hardened/Custom kernels must use the DKMS variant.
static std::string official_driver_for_kernel(KernelType k) {
    switch (k) {
    case KernelType::Linux:
        return "nvidia-open";
    case KernelType::LinuxLts:
        return "nvidia-open-lts";
    default:
        return "nvidia-open-dkms";
    }
}

std::string SystemInfo::driver_for_arch(NvidiaArch arch) {
    switch (arch) {
    case NvidiaArch::Blackwell:
    case NvidiaArch::AdaLovelace:
    case NvidiaArch::Ampere:
    case NvidiaArch::Turing:
        return "nvidia-open";
    case NvidiaArch::Volta:
    case NvidiaArch::Pascal:
    case NvidiaArch::Maxwell:
        return "nvidia-580xx-dkms";
    case NvidiaArch::Kepler:
        return "nvidia-470xx-dkms";
    case NvidiaArch::Fermi:
        return "nvidia-390xx-dkms";
    case NvidiaArch::Tesla:
        return "nvidia-340xx-dkms";
    default:
        return "nvidia-open";
    }
}

std::string SystemInfo::utils_for_arch(NvidiaArch arch) {
    switch (arch) {
    case NvidiaArch::Blackwell:
    case NvidiaArch::AdaLovelace:
    case NvidiaArch::Ampere:
    case NvidiaArch::Turing:
        return "nvidia-utils";
    case NvidiaArch::Volta:
    case NvidiaArch::Pascal:
    case NvidiaArch::Maxwell:
        return "nvidia-580xx-utils";
    case NvidiaArch::Kepler:
        return "nvidia-470xx-utils";
    case NvidiaArch::Fermi:
        return "nvidia-390xx-utils";
    default:
        return "";
    }
}

std::string SystemInfo::lib32_for_arch(NvidiaArch arch) {
    switch (arch) {
    case NvidiaArch::Blackwell:
    case NvidiaArch::AdaLovelace:
    case NvidiaArch::Ampere:
    case NvidiaArch::Turing:
        return "lib32-nvidia-utils";
    case NvidiaArch::Volta:
    case NvidiaArch::Pascal:
    case NvidiaArch::Maxwell:
        return "lib32-nvidia-580xx-utils";
    case NvidiaArch::Kepler:
        return "lib32-nvidia-470xx-utils";
    default:
        return "";
    }
}

std::optional<GpuInfo> SystemInfo::detect_nvidia() {
    auto res = utils::exec("lspci -k -d ::03xx");
    if (res.exit_code != 0)
        return std::nullopt;

    for (auto& line : utils::split(res.stdout_str, '\n')) {
        std::string l = to_lower(line);
        if (l.find("nvidia") == std::string::npos)
            continue;

        GpuInfo gpu;
        gpu.name = utils::trim(line);

        // Extract PCI id (first field like "01:00.0")
        auto parts = utils::split(utils::trim(line), ' ');
        if (!parts.empty())
            gpu.pci_id = parts[0];

        gpu.arch = arch_from_name(gpu.name);
        gpu.driver_package = driver_for_arch(gpu.arch);
        gpu.utils_package = utils_for_arch(gpu.arch);
        gpu.lib32_package = lib32_for_arch(gpu.arch);
        gpu.driver_is_aur = driver_is_aur(gpu.arch);
        return gpu;
    }
    return std::nullopt;
}

IgpuVendor SystemInfo::detect_igpu() {
    auto res = utils::exec("lspci -k -d ::03xx");
    if (res.exit_code != 0)
        return IgpuVendor::None;

    bool has_intel = false, has_amd = false;
    for (auto& line : utils::split(res.stdout_str, '\n')) {
        std::string l = to_lower(line);
        if (l.find("intel") != std::string::npos)
            has_intel = true;
        if (l.find("amd") != std::string::npos || l.find("advanced micro") != std::string::npos)
            has_amd = true;
    }
    if (has_intel)
        return IgpuVendor::Intel;
    if (has_amd)
        return IgpuVendor::Amd;
    return IgpuVendor::None;
}

KernelType SystemInfo::detect_kernel(std::string& ver_out) {
    auto res = utils::exec("uname -r");
    ver_out = utils::trim(res.stdout_str);
    std::string v = to_lower(ver_out);
    if (v.find("-lts") != std::string::npos)
        return KernelType::LinuxLts;
    if (v.find("-zen") != std::string::npos)
        return KernelType::LinuxZen;
    if (v.find("-hardened") != std::string::npos)
        return KernelType::LinuxHardened;
    if (v.find("arch") != std::string::npos)
        return KernelType::Linux;
    return KernelType::Custom;
}

Bootloader SystemInfo::detect_bootloader() {
    if (utils::file_exists("/etc/default/grub"))
        return Bootloader::Grub;
    if (utils::file_exists("/boot/loader/loader.conf"))
        return Bootloader::SystemdBoot;
    if (utils::file_exists("/boot/efi/EFI/refind"))
        return Bootloader::Refind;
    return Bootloader::Unknown;
}

bool SystemInfo::driver_is_aur(NvidiaArch arch) {
    switch (arch) {
    case NvidiaArch::Blackwell:
    case NvidiaArch::AdaLovelace:
    case NvidiaArch::Ampere:
    case NvidiaArch::Turing:
        return false; // nvidia-open is in official repos
    default:
        return true; // all dkms legacy drivers are AUR
    }
}

AurHelper SystemInfo::detect_aur_helper() {
    if (utils::exec("command -v paru").exit_code == 0)
        return AurHelper::Paru;
    if (utils::exec("command -v yay").exit_code == 0)
        return AurHelper::Yay;
    return AurHelper::None;
}

SessionType SystemInfo::detect_session() {
    // $XDG_SESSION_TYPE is stripped by sudo; query loginctl instead.
    auto res = utils::exec(
        "loginctl show-session $(loginctl | awk 'NR==2{print $1}') -p Type --value 2>/dev/null");
    std::string s = utils::trim(res.stdout_str);
    if (s == "wayland")
        return SessionType::Wayland;
    if (s == "x11")
        return SessionType::X11;
    // Fallback: scan all processes of the real user for XDG_SESSION_TYPE.
    // Works for any DE (GNOME, KDE, Hyprland, Sway, etc.) unlike checking a specific process name.
    auto env = utils::exec(
        "loginname=$(logname 2>/dev/null) && "
        "for pid in $(pgrep -u \"$loginname\" 2>/dev/null); do "
        "val=$(tr '\\0' '\\n' < /proc/$pid/environ 2>/dev/null | grep '^XDG_SESSION_TYPE='); "
        "[ -n \"$val\" ] && echo \"$val\" && break; done");
    s = utils::trim(env.stdout_str);
    if (s.find("wayland") != std::string::npos)
        return SessionType::Wayland;
    if (s.find("x11") != std::string::npos)
        return SessionType::X11;
    return SessionType::Unknown;
}

SystemInfo SystemInfo::detect() {
    SystemInfo info;
    info.nvidia_gpu = detect_nvidia();
    info.igpu_vendor = detect_igpu();
    info.is_optimus = info.nvidia_gpu.has_value() && info.igpu_vendor != IgpuVendor::None;
    info.kernel = detect_kernel(info.kernel_ver);
    info.bootloader = detect_bootloader();
    info.session = detect_session();
    info.aur_helper = detect_aur_helper();

    // For official (Turing+) drivers, pick the right pre-built vs DKMS variant now that
    // the kernel type is known. AUR legacy drivers always use DKMS so no adjustment needed.
    if (info.nvidia_gpu && !info.nvidia_gpu->driver_is_aur)
        info.nvidia_gpu->driver_package = official_driver_for_kernel(info.kernel);

    return info;
}
