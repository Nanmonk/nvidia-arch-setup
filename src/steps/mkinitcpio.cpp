#include "mkinitcpio.h"

#include "core/utils.h"

#include <regex>

static const std::string CONF = "/etc/mkinitcpio.conf";
static const std::string MODULES_NEEDED = "nvidia nvidia_modeset nvidia_uvm nvidia_drm";

std::string MkinitcpioStep::description() const {
    return "Add nvidia modules to MODULES array and remove 'kms' from HOOKS in "
           "/etc/mkinitcpio.conf";
}

std::string MkinitcpioStep::preview(const SystemInfo& /* info */) const {
    auto content = utils::read_file(CONF);
    if (!content)
        return "Cannot read " + CONF;

    std::string result = "Changes to " + CONF + ":\n";
    if (content->find("nvidia") == std::string::npos)
        result += "  MODULES: add " + MODULES_NEEDED + "\n";
    else
        result += "  MODULES: nvidia modules already present\n";

    if (content->find("kms") != std::string::npos)
        result += "  HOOKS: remove 'kms'";
    else
        result += "  HOOKS: 'kms' not present, no change needed";
    return result;
}

bool MkinitcpioStep::applicable(const SystemInfo& info) const {
    return info.nvidia_gpu.has_value();
}

bool MkinitcpioStep::execute(const SystemInfo& /* info */) {
    auto content = utils::read_file(CONF);
    if (!content) {
        utils::print_err("Cannot read " + CONF);
        return false;
    }

    std::string text = *content;

    // Add nvidia modules to MODULES=()
    std::regex modules_re(R"(^MODULES=\(([^)]*)\))", std::regex::multiline);
    std::smatch m;
    if (std::regex_search(text, m, modules_re)) {
        std::string existing = m[1].str();
        // Check each needed module
        for (const auto& mod : utils::split(MODULES_NEEDED, ' ')) {
            if (existing.find(mod) == std::string::npos) {
                existing += (existing.empty() ? "" : " ") + mod;
            }
        }
        text = std::regex_replace(text, modules_re, "MODULES=(" + existing + ")");
    } else {
        utils::print_err("MODULES= line not found in " + CONF);
        return false;
    }

    // Remove 'kms' from HOOKS=()
    std::regex hooks_re(R"(^HOOKS=\(([^)]*)\))", std::regex::multiline);
    if (std::regex_search(text, m, hooks_re)) {
        std::string hooks = m[1].str();
        // Remove 'kms' word
        hooks = std::regex_replace(hooks, std::regex(R"(\bkms\b\s*)"), "");
        hooks = utils::trim(hooks);
        text = std::regex_replace(text, hooks_re, "HOOKS=(" + hooks + ")");
    }

    if (!utils::write_file(CONF, text)) {
        utils::print_err("Cannot write " + CONF);
        return false;
    }

    utils::print_ok("mkinitcpio.conf updated");
    return true;
}

bool MkinitcpioRebuildStep::execute(const SystemInfo& /* info */) {
    return utils::exec_interactive("mkinitcpio -P");
}
