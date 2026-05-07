#pragma once
#include <string>
#include <vector>
#include <optional>

namespace utils {

struct ExecResult {
    int exit_code;
    std::string stdout_str;
    std::string stderr_str;
};

ExecResult exec(const std::string& cmd);
bool exec_interactive(const std::string& cmd);

bool write_file(const std::string& path, const std::string& content);
std::optional<std::string> read_file(const std::string& path);
bool file_contains(const std::string& path, const std::string& str);
bool file_exists(const std::string& path);

std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s, char delim);

void print_ok(const std::string& msg);
void print_err(const std::string& msg);
void print_info(const std::string& msg);
void print_warn(const std::string& msg);

} // namespace utils
