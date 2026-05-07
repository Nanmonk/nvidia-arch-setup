#include "kernel_params.hpp"
#include "core/utils.hpp"
#include <filesystem>
#include <regex>

static const std::string PARAMS = "nvidia_drm.modeset=1 nvidia_drm.fbdev=1";

std::string KernelParamsStep::description() const {
    return "Add '" + PARAMS + "' to bootloader kernel parameters";
}

bool KernelParamsStep::applicable(const SystemInfo& info) const {
    return info.nvidia_gpu.has_value() &&
           (info.bootloader == Bootloader::Grub ||
            info.bootloader == Bootloader::SystemdBoot);
}

bool KernelParamsStep::apply_grub(const SystemInfo& /* info */) {
    const std::string path = "/etc/default/grub";
    auto content = utils::read_file(path);
    if (!content) { utils::print_err("Cannot read " + path); return false; }

    std::string text = *content;
    std::regex re(R"re(^(GRUB_CMDLINE_LINUX_DEFAULT="[^"]*))re", std::regex::multiline);
    std::smatch m;
    if (!std::regex_search(text, m, re)) {
        utils::print_err("GRUB_CMDLINE_LINUX_DEFAULT not found");
        return false;
    }

    std::string line = m[1].str();
    for (const auto& param : utils::split(PARAMS, ' ')) {
        if (line.find(param) == std::string::npos)
            line += " " + param;
    }
    line += "\"";
    text = std::regex_replace(text, re, line);

    if (!utils::write_file(path, text)) { utils::print_err("Cannot write " + path); return false; }
    utils::print_info("Regenerating GRUB config...");
    return utils::exec_interactive("grub-mkconfig -o /boot/grub/grub.cfg");
}

bool KernelParamsStep::apply_systemd_boot(const SystemInfo& /* info */) {
    namespace fs = std::filesystem;
    const std::string entries_dir = "/boot/loader/entries";

    bool any = false;
    for (auto& entry : fs::directory_iterator(entries_dir)) {
        if (entry.path().extension() != ".conf") continue;

        auto content = utils::read_file(entry.path().string());
        if (!content) continue;

        std::string text = *content;
        std::regex re(R"(^(options\s+.*)$)", std::regex::multiline);
        std::smatch m;
        if (!std::regex_search(text, m, re)) continue;

        std::string options_line = m[1].str();
        bool changed = false;
        for (const auto& param : utils::split(PARAMS, ' ')) {
            if (options_line.find(param) == std::string::npos) {
                options_line += " " + param;
                changed = true;
            }
        }
        if (changed) {
            text = std::regex_replace(text, re, options_line);
            utils::write_file(entry.path().string(), text);
            utils::print_ok("Updated: " + entry.path().filename().string());
        } else {
            utils::print_info("Already set: " + entry.path().filename().string());
        }
        any = true;
    }
    return any;
}

bool KernelParamsStep::execute(const SystemInfo& info) {
    switch (info.bootloader) {
        case Bootloader::Grub:        return apply_grub(info);
        case Bootloader::SystemdBoot: return apply_systemd_boot(info);
        default:
            utils::print_warn("Unknown bootloader — add manually: " + PARAMS);
            return false;
    }
}
