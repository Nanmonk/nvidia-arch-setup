# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 构建

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

二进制文件输出至 `build/nvidia-arch-setup`。`compile_commands.json` 自动生成（clangd 所需）。

**无需 root / 无硬件时测试：**
```bash
./build/nvidia-arch-setup --dry-run
```

**正式运行（需要 root）：**
```bash
sudo ./build/nvidia-arch-setup [--default] [--dry-run]
```

## 架构

程序是一条顺序执行的配置步骤流水线，每个步骤由 `applicable()` 决定是否在当前机器上运行。所有系统检测在 `SystemInfo::detect()` 中一次性完成，结果以只读方式传给每个步骤。

```
main()
  └─ SystemInfo::detect()          # 一次性检测，不会重复执行
  └─ Runner::run(info)
        ├─ Step::applicable(info)  # 判断步骤是否适用于当前机器
        ├─ Step::preview(info)     # 仅用于 --dry-run（无副作用）
        └─ Step::execute(info)     # 实际修改系统
```

### 添加新步骤

1. 创建 `src/steps/foo.hpp` 和 `src/steps/foo.cpp`
2. 继承 `Step`（定义于 `src/core/step.hpp`），实现 `name()`、`description()`、`execute()`；按需覆写 `applicable()` 和 `preview()`
3. 将 `.cpp` 加入 `CMakeLists.txt`
4. 在 `main.cpp` 中用 `runner.add(std::make_unique<FooStep>())` 注册

### 关键数据流

- `SystemInfo`（定义于 `src/core/system_info.hpp`）保存所有检测结果：`nvidia_gpu`（`optional<GpuInfo>`）、`is_optimus`、`kernel`、`bootloader`、`session`、`aur_helper`。

- **驱动包选择**（`system_info.cpp`）：`detect_nvidia()` 先用 `driver_for_arch()` 写入占位值，`detect()` 在知道内核类型后，对 Turing+ 调用 `official_driver_for_kernel()` 覆写：
  - `linux` → `nvidia-open`
  - `linux-lts` → `nvidia-open-lts`
  - zen / hardened / 自定义 → `nvidia-open-dkms`
  - Pascal/Maxwell/Volta → `nvidia-580xx-dkms`（AUR，不覆写）
  - Kepler → `nvidia-470xx-dkms`（AUR）

- `GpuInfo.driver_is_aur`：Turing+ 为 false（官方源），Pascal 及更旧为 true（AUR）。

- **`DriverInstallStep` 包拆分逻辑**（重要）：
  - 官方驱动（`driver_is_aur = false`）：`pacman -S <driver> nvidia-utils [lib32-nvidia-utils]`，全部走官方源
  - AUR 遗留驱动（`driver_is_aur = true`）：`pacman -S <kernel-headers>`（DKMS 编译依赖），然后 `paru -S <driver> [lib32]`。**不能**先装 `nvidia-utils`，因为它与 AUR 的 `nvidia-580xx-utils` 冲突

- `KernelParamsStep` 编辑 `/etc/default/grub`（正则匹配 `GRUB_CMDLINE_LINUX_DEFAULT`）或遍历 `/boot/loader/entries/*.conf`（systemd-boot）。**不支持 rEFInd**（`applicable()` 直接返回 false）。

- `MkinitcpioStep` 编辑 `/etc/mkinitcpio.conf`（向 `MODULES=()` 添加 nvidia 模块，从 `HOOKS=()` 移除 `kms`）。`MkinitcpioRebuildStep` 是独立步骤，在流水线末尾执行 `mkinitcpio -P`。

- `OptimusStep` 仅在 `info.is_optimus` 为 true 时运行，安装 `nvidia-prime xorg-xrandr`，写入 Xorg outputclass 配置，修补 SDDM 的 `Xsetup`，创建 `/usr/local/bin/prime-run`。

- `NvidiaSuspendStep` 启用 `nvidia-suspend/resume/hibernate.service`，防止休眠黑屏。首次安装驱动后、重启前 enable 可能失败（服务尚不存在），这是正常的。

- `PacmanHookStep` 根据检测到的驱动包名写入 `/etc/pacman.d/hooks/nvidia.hook`。

### 工具函数

`src/core/utils.hpp` 提供：
- `exec()` — 捕获 stdout/stderr，非交互式
- `exec_interactive()` — 使用 `std::system()`，允许 pacman/paru 向用户提问；记录命令和退出码
- 文件 I/O：`read_file`、`write_file`、`file_contains`、`file_exists`
- `print_ok/err/info/warn/raw` — 同时写入 stdout 和日志文件
- `log_init(path)` — 启动时调用一次（`--dry-run` 下跳过），追加写入 `/var/log/nvidia-arch-setup.log`

## 代码风格

由 `.clang-format` 强制执行（基于 LLVM，C++17）：
- 4 空格缩进，不用 Tab
- 每行最多 100 列
- `Type*` 和 `Type&`（指针/引用符号靠左）
- 本地头文件（`"..."`）排在系统头文件（`<...>`）之前

包含 `)"` 的正则原始字符串字面量必须使用自定义分隔符以防提前终止：`R"re(...)re"`（参见 `kernel_params.cpp`）。

## 本地打包（PKGBUILD）

项目根目录有 `PKGBUILD`，可用于本地 `makepkg` 安装：

```bash
# 本地构建安装（修改 PKGBUILD 中 source 行为 git+file:// 路径）
makepkg -si
```

正式发布到 AUR 时，将 `source` 中的 URL 替换为 GitHub release 链接，并填入正确的 `sha256sums`。
