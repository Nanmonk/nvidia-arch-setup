#pragma once
#include <optional>
#include <string>
#include <vector>

enum class NvidiaArch {
    Blackwell,   // GBXXX          -> nvidia-open
    AdaLovelace, // NV190/ADXXX    -> nvidia-open
    Ampere,      // NV170/GAXXX    -> nvidia-open
    Turing,      // NV160/TUXXX    -> nvidia-open
    Volta,       // NV140/GV100    -> nvidia-580xx-dkms
    Pascal,      // NV130/GPXXX    -> nvidia-580xx-dkms
    Maxwell,     // NV110/GMXXX    -> nvidia-580xx-dkms
    Kepler,      // NVE0/GKXXX     -> nvidia-470xx-dkms
    Fermi,       // NVC0/GF1XX     -> nvidia-390xx-dkms
    Tesla,       // NV50           -> nvidia-340xx-dkms
    Unknown
};

enum class KernelType { Linux, LinuxLts, LinuxZen, LinuxHardened, Custom };
enum class Bootloader { Grub, SystemdBoot, Refind, Unknown };
enum class SessionType { Wayland, X11, Unknown };
enum class IgpuVendor { Intel, Amd, None };
enum class AurHelper { Paru, Yay, None };

struct GpuInfo {
    std::string pci_id;
    std::string name;
    NvidiaArch arch;
    std::string driver_package;
    std::string utils_package;
    std::string lib32_package;
    bool driver_is_aur = false;
};

struct SystemInfo {
    std::optional<GpuInfo> nvidia_gpu;
    IgpuVendor igpu_vendor = IgpuVendor::None;
    bool is_optimus = false;
    KernelType kernel = KernelType::Linux;
    std::string kernel_ver;
    Bootloader bootloader = Bootloader::Unknown;
    SessionType session = SessionType::Unknown;
    AurHelper aur_helper = AurHelper::None;

    static SystemInfo detect();

  private:
    static std::optional<GpuInfo> detect_nvidia();
    static IgpuVendor detect_igpu();
    static KernelType detect_kernel(std::string& ver_out);
    static Bootloader detect_bootloader();
    static SessionType detect_session();
    static NvidiaArch arch_from_name(const std::string& name);
    static std::string driver_for_arch(NvidiaArch arch);
    static std::string utils_for_arch(NvidiaArch arch);
    static std::string lib32_for_arch(NvidiaArch arch);
    static bool driver_is_aur(NvidiaArch arch);
    static AurHelper detect_aur_helper();
};
