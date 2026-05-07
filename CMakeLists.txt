cmake_minimum_required(VERSION 3.16)
project(nvidia-arch-setup VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(nvidia-arch-setup
    src/main.cpp
    src/core/system_info.cpp
    src/core/runner.cpp
    src/core/utils.cpp
    src/steps/driver_install.cpp
    src/steps/mkinitcpio.cpp
    src/steps/kernel_params.cpp
    src/steps/pacman_hook.cpp
    src/steps/optimus.cpp
    src/steps/suspend.cpp
)

target_include_directories(nvidia-arch-setup PRIVATE src)
target_compile_options(nvidia-arch-setup PRIVATE -Wall -Wextra)
