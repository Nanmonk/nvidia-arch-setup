#include "utils.hpp"
#include <array>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/wait.h>

namespace utils {

ExecResult exec(const std::string& cmd) {
    ExecResult result;
    std::array<char, 256> buf;

    std::string full_cmd = cmd + " 2>/tmp/nvidia_setup_stderr";
    FILE* pipe = popen(full_cmd.c_str(), "r");
    if (!pipe) {
        result.exit_code = -1;
        return result;
    }
    while (fgets(buf.data(), buf.size(), pipe))
        result.stdout_str += buf.data();
    result.exit_code = WEXITSTATUS(pclose(pipe));

    auto err = read_file("/tmp/nvidia_setup_stderr");
    if (err) result.stderr_str = *err;

    return result;
}

bool exec_interactive(const std::string& cmd) {
    return WEXITSTATUS(std::system(cmd.c_str())) == 0;
}

bool write_file(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f) return false;
    f << content;
    return f.good();
}

std::optional<std::string> read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) return std::nullopt;
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

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end   = s.find_last_not_of(" \t\n\r");
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

void print_ok(const std::string& msg)   { std::cout << "\033[32m[✓]\033[0m " << msg << "\n"; }
void print_err(const std::string& msg)  { std::cerr << "\033[31m[✗]\033[0m " << msg << "\n"; }
void print_info(const std::string& msg) { std::cout << "\033[34m[i]\033[0m " << msg << "\n"; }
void print_warn(const std::string& msg) { std::cout << "\033[33m[!]\033[0m " << msg << "\n"; }

} // namespace utils
