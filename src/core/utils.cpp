#include "utils.h"

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

namespace utils {

// --- logging ---

static std::ofstream g_log;
static std::string g_log_path;

static std::string timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

void log_init(const std::string& path) {
    g_log_path = path;
    g_log.open(path, std::ios::app);
    if (g_log)
        g_log << "\n=== nvidia-arch-setup " << timestamp() << " ===\n";
}

void log_write(const std::string& level, const std::string& msg) {
    if (!g_log)
        return;
    g_log << "[" << timestamp() << "] [" << level << "] " << msg << "\n";
    g_log.flush();
}

std::string log_path() {
    return g_log_path;
}

// --- exec ---

ExecResult exec(const std::string& cmd) {
    ExecResult result;
    std::array<char, 256> buf;

    // Use mkstemp to avoid permission conflicts when switching between
    // root and non-root (e.g. dry-run as user, then sudo run as root).
    char tmppath[] = "/tmp/nvidia_setup_XXXXXX";
    int tmpfd = mkstemp(tmppath);
    if (tmpfd == -1) {
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            result.exit_code = -1;
            return result;
        }
        while (fgets(buf.data(), buf.size(), pipe))
            result.stdout_str += buf.data();
        result.exit_code = WEXITSTATUS(pclose(pipe));
        return result;
    }
    close(tmpfd);

    std::string full_cmd = cmd + " 2>" + tmppath;
    FILE* pipe = popen(full_cmd.c_str(), "r");
    if (!pipe) {
        unlink(tmppath);
        result.exit_code = -1;
        return result;
    }
    while (fgets(buf.data(), buf.size(), pipe))
        result.stdout_str += buf.data();
    result.exit_code = WEXITSTATUS(pclose(pipe));

    auto err = read_file(tmppath);
    if (err)
        result.stderr_str = *err;
    unlink(tmppath);

    return result;
}

bool exec_interactive(const std::string& cmd) {
    log_write("CMD", cmd);
    int status = std::system(cmd.c_str());
    // WIFEXITED is false when killed by signal (e.g. Ctrl+C); treat as failure.
    bool ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;
    log_write("CMD", std::string("exit=") + (WIFEXITED(status) ? std::to_string(WEXITSTATUS(status)) : "signal"));
    return ok;
}

// --- file ops ---

bool write_file(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f)
        return false;
    f << content;
    return f.good();
}

std::optional<std::string> read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f)
        return std::nullopt;
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool file_contains(const std::string& path, const std::string& str) {
    auto content = read_file(path);
    return content && content->find(str) != std::string::npos;
}

bool file_exists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

// --- string ---

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::istringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delim))
        parts.push_back(token);
    return parts;
}

// --- print ---

void print_ok(const std::string& msg) {
    std::cout << "\033[32m[✓]\033[0m " << msg << "\n";
    log_write("OK", msg);
}
void print_err(const std::string& msg) {
    std::cerr << "\033[31m[✗]\033[0m " << msg << "\n";
    log_write("ERR", msg);
}
void print_info(const std::string& msg) {
    std::cout << "\033[34m[i]\033[0m " << msg << "\n";
    log_write("INFO", msg);
}
void print_warn(const std::string& msg) {
    std::cout << "\033[33m[!]\033[0m " << msg << "\n";
    log_write("WARN", msg);
}
void print_raw(const std::string& msg) {
    std::cout << msg << "\n";
    log_write("INFO", msg);
}

} // namespace utils
