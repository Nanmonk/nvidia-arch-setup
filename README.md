# nvidia-arch-setup

Automatic NVIDIA driver configurator for Arch Linux, based on the [Arch Wiki](https://wiki.archlinux.org/title/NVIDIA) and [NVIDIA Optimus](https://wiki.archlinux.org/title/NVIDIA_Optimus) documentation.

## Features

- Auto-detects GPU model and selects the correct driver package
- Supports Optimus (hybrid Intel/AMD + NVIDIA) via PRIME render offload
- Configures mkinitcpio modules and hooks
- Sets kernel parameters for DRM/Wayland (`nvidia_drm.modeset=1`, `nvidia_drm.fbdev=1`)
- Installs pacman hook to auto-rebuild initramfs on driver updates
- Creates `prime-run` wrapper for per-app GPU selection on Optimus systems
- Interactive mode (confirm each step) or default mode (fully automated)

## Supported Drivers

| GPU Architecture | Driver Package |
|---|---|
| Blackwell / Ada Lovelace / Ampere / Turing | `nvidia-open` |
| Volta / Pascal / Maxwell | `nvidia-580xx-dkms` |
| Kepler | `nvidia-470xx-dkms` |
| Fermi | `nvidia-390xx-dkms` |
| Tesla | `nvidia-340xx-dkms` |

## Requirements

- Arch Linux
- CMake >= 3.16
- GCC or Clang with C++17 support

```bash
sudo pacman -S cmake base-devel
```

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Usage

```bash
# Interactive mode — confirms each step before executing
sudo ./build/nvidia-arch-setup

# Default mode — runs all applicable steps without prompts
sudo ./build/nvidia-arch-setup --default
```

## Steps

| Step | Description |
|---|---|
| Install NVIDIA Driver | Installs driver, `nvidia-utils`, lib32, and kernel headers (for dkms) |
| Configure mkinitcpio | Adds `nvidia nvidia_modeset nvidia_uvm nvidia_drm` to `MODULES`, removes `kms` from `HOOKS` |
| Set Kernel Parameters | Adds `nvidia_drm.modeset=1 nvidia_drm.fbdev=1` to GRUB or systemd-boot |
| Install Pacman Hook | Creates `/etc/pacman.d/hooks/nvidia.hook` to rebuild initramfs on updates |
| Configure Optimus | Sets up PRIME render offload, writes Xorg outputclass config, creates `prime-run` |
| Rebuild initramfs | Runs `mkinitcpio -P` |

## Optimus / PRIME

On Optimus systems, after setup you can run any application on the discrete NVIDIA GPU with:

```bash
prime-run <application>
```

Or set environment variables manually:

```bash
__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia <application>
```

## Bootloader Support

| Bootloader | Method |
|---|---|
| GRUB | Appends params to `GRUB_CMDLINE_LINUX_DEFAULT`, runs `grub-mkconfig` |
| systemd-boot | Appends params to all `options` lines in `/boot/loader/entries/*.conf` |
| rEFInd | Not yet supported — add params manually |

## After Setup

Reboot to apply all changes:

```bash
sudo reboot
```

Verify DRM modeset is active:

```bash
cat /sys/module/nvidia_drm/parameters/modeset   # should print Y
cat /sys/module/nvidia_drm/parameters/fbdev      # should print Y
```
