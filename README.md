# nvidia-arch-setup

自动配置 Arch Linux 上 NVIDIA 驱动的命令行工具，基于 [Arch Wiki NVIDIA](https://wiki.archlinux.org/title/NVIDIA) 和 [NVIDIA Optimus](https://wiki.archlinux.org/title/NVIDIA_Optimus)。

[![GitHub](https://img.shields.io/badge/GitHub-Nanmonk%2Fnvidia--arch--setup-blue)](https://github.com/Nanmonk/nvidia-arch-setup)

## 功能

- 自动检测 GPU 型号，选择正确的驱动包
- 根据内核类型选择预编译包或 DKMS 变体
- 支持 Optimus 双显卡（Intel/AMD + NVIDIA）PRIME 直通
- 配置 mkinitcpio 模块和 hooks
- 写入内核参数（`nvidia_drm.modeset=1`、`nvidia_drm.fbdev=1`）
- 安装 pacman hook，驱动升级后自动重建 initramfs
- 启用 nvidia-suspend/resume/hibernate 服务（防止休眠黑屏）
- 交互式 / 默认 / 预览（dry-run）三种运行模式

## 驱动选择逻辑

工具根据 GPU 架构自动选择驱动包，再根据内核类型决定安装预编译版还是 DKMS 版。

### 支持的 GPU 型号与驱动包

| 架构 | 代表显卡型号 | 驱动包 | 来源 |
|---|---|---|---|
| **Blackwell** (50xx) | RTX 5090 / 5080 / 5070 / 5060 | `nvidia-open` / `nvidia-open-lts` / `nvidia-open-dkms` | 官方源 |
| **Ada Lovelace** (40xx) | RTX 4090 / 4080 / 4070 / 4060 | `nvidia-open` / `nvidia-open-lts` / `nvidia-open-dkms` | 官方源 |
| **Ampere** (30xx / A系) | RTX 3090 / 3080 / 3070 / 3060 / A6000 | `nvidia-open` / `nvidia-open-lts` / `nvidia-open-dkms` | 官方源 |
| **Turing** (20xx / 16xx) | RTX 2080 / 2070 / 2060 / GTX 1660 / 1650 | `nvidia-open` / `nvidia-open-lts` / `nvidia-open-dkms` | 官方源 |
| **Volta** (V系) | Titan V / V100 | `nvidia-580xx-dkms` | AUR |
| **Pascal** (10xx) | GTX 1080 / 1070 / 1060 / 1050 / Titan X | `nvidia-580xx-dkms` | AUR |
| **Maxwell** (9xx / 8xx / 750) | GTX 980 / 970 / 960 / 950 / 750 Ti | `nvidia-580xx-dkms` | AUR |
| **Kepler** (7xx / 6xx) | GTX 780 / 770 / 760 / 680 / 670 / 660 | `nvidia-470xx-dkms` | AUR |
| **Fermi** (5xx / 4xx) | GTX 580 / 570 / 560 / 480 / 470 | `nvidia-390xx-dkms` | AUR |
| **Tesla** (GT 200) | GT 240 / 220 / 9800 GTX | `nvidia-340xx-dkms` | AUR |

> **注意**：Pascal 及更旧架构（GTX 10xx 及以下）从 NVIDIA 595 驱动起已移出官方支持，必须使用 AUR 遗留驱动，且需要提前安装 AUR 助手（paru / yay）。

### 官方驱动根据内核自动选择

对于 Turing+ 架构（官方源驱动），工具根据当前内核选择最合适的包：

| 内核 | 驱动包 | 说明 |
|---|---|---|
| `linux` | `nvidia-open` | 预编译，即装即用 |
| `linux-lts` | `nvidia-open-lts` | 预编译，即装即用 |
| `linux-zen` / `linux-hardened` / 自定义 | `nvidia-open-dkms` | 本地 DKMS 编译，约 5~15 分钟 |

## 安装

### 从 GitHub 克隆构建（推荐）

```bash
# 克隆仓库
git clone https://github.com/Nanmonk/nvidia-arch-setup.git
cd nvidia-arch-setup

# 安装构建依赖
sudo pacman -S cmake base-devel pciutils

# 构建
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 运行
sudo ./build/nvidia-arch-setup
```

### 使用 PKGBUILD 安装（安装到系统路径）

```bash
git clone https://github.com/Nanmonk/nvidia-arch-setup.git
cd nvidia-arch-setup
makepkg -si
# 安装后可直接运行：
sudo nvidia-arch-setup
```

### 前置依赖

如果你的 GPU 是 Pascal 及更旧架构（GTX 10xx 及以下），还需要 AUR 助手：

```bash
# 安装 paru（推荐）
sudo pacman -S --needed base-devel git
git clone https://aur.archlinux.org/paru.git
cd paru && makepkg -si
```

## 使用

```bash
# 交互模式 — 每步执行前确认
sudo ./build/nvidia-arch-setup

# 默认模式 — 全自动，无确认提示
sudo ./build/nvidia-arch-setup --default

# 预览模式 — 只显示将要执行的操作，不修改系统（无需 root）
./build/nvidia-arch-setup --dry-run
```

## 配置步骤

| 步骤 | 描述 |
|---|---|
| Install NVIDIA Driver | 安装驱动包、utils、lib32；Pascal 及更旧通过 AUR 安装 |
| Configure mkinitcpio | 添加 `nvidia nvidia_modeset nvidia_uvm nvidia_drm` 到 MODULES，移除 `kms` |
| Set Kernel Parameters | 向 GRUB 或 systemd-boot 写入 `nvidia_drm.modeset=1 nvidia_drm.fbdev=1` |
| Install Pacman Hook | 写入 `/etc/pacman.d/hooks/nvidia.hook`，驱动更新后自动重建 initramfs |
| Configure Optimus | 写入 Xorg outputclass 配置，安装 nvidia-prime，创建 prime-run wrapper |
| Enable Suspend Services | 启用 nvidia-suspend / nvidia-resume / nvidia-hibernate |
| Rebuild initramfs | 执行 `mkinitcpio -P` |

## Optimus / PRIME

Optimus 配置完成后，用 `prime-run` 在 NVIDIA 独显上运行应用：

```bash
prime-run glxinfo | grep "OpenGL renderer"
prime-run vulkaninfo | grep "GPU"
prime-run steam
prime-run blender
```

也可以手动设置环境变量：

```bash
__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia <应用>
```

## 支持的引导器

| 引导器 | 处理方式 |
|---|---|
| GRUB | 修改 `GRUB_CMDLINE_LINUX_DEFAULT`，运行 `grub-mkconfig` |
| systemd-boot | 修改 `/boot/loader/entries/*.conf` 中所有 `options` 行 |
| rEFInd | 不支持，需手动添加参数 |

## 注意事项

### Pascal 及更旧 GPU（GTX 10xx 及以下）

- 官方源已移除闭源 `nvidia-dkms`，从 595 版本起 Pascal 必须使用 AUR 的 `nvidia-580xx-dkms`
- `nvidia-580xx-utils` 与官方 `nvidia-utils` 冲突。若已安装 `nvidia-utils`，paru 会提示替换，选 Y 即可
- 安装前建议先卸载旧驱动：`sudo pacman -Rns nvidia-utils lib32-nvidia-utils`（如已安装）

### DKMS 编译时间

- `nvidia-open-dkms`（zen/hardened 内核）和 `nvidia-580xx-dkms` 均需本地编译，约需 5~15 分钟

### Suspend/Resume 服务

- 若驱动尚未加载（首次安装后重启前），`systemctl enable` 可能报错，重启后再手动 enable 即可：
  ```bash
  sudo systemctl enable nvidia-suspend.service nvidia-resume.service nvidia-hibernate.service
  ```

### Wayland

- GTX 10xx（Pascal）在 Wayland 下基本可用，但稳定性不及 Turing+ 架构
- 推荐桌面环境：KDE Plasma Wayland（对 NVIDIA 支持最好）
- Electron 应用（VSCode、Discord）需加 `--enable-wayland-ime` 才能使用输入法

### 已有安装

- 若系统已有部分 NVIDIA 配置，建议先用 `--dry-run` 预览，确认无冲突后再正式运行

## 重启后验证

```bash
# 驱动是否加载
nvidia-smi

# 内核模块
lsmod | grep nvidia

# DRM modeset 是否生效
cat /sys/module/nvidia_drm/parameters/modeset   # 应输出 Y
cat /sys/module/nvidia_drm/parameters/fbdev      # 应输出 Y

# Optimus：在 NVIDIA 上运行测试
prime-run glxinfo | grep "OpenGL renderer"
```

## 日志

运行日志写入 `/var/log/nvidia-arch-setup.log`，可用于排查问题：

```bash
cat /var/log/nvidia-arch-setup.log
```
