# Maintainer: Nanmonk <nanmonk2026@gmail.com>
pkgname=nvidia-arch-setup
pkgver=1.0.0
pkgrel=1
pkgdesc='Automatic NVIDIA driver configurator for Arch Linux'
arch=('x86_64')
url='https://github.com/Nanmonk/nvidia-arch-setup'
license=('MIT')
makedepends=('cmake' 'gcc')
depends=('pciutils')
optdepends=(
    'paru: AUR helper required for Pascal/Kepler/Fermi legacy drivers'
    'yay: AUR helper required for Pascal/Kepler/Fermi legacy drivers'
)
# Local build: replace source with the line below and run makepkg -si
# source=("$pkgname::git+file:///home/Nanmon/Tools/nvidia-arch-setup")
source=("$pkgname-$pkgver.tar.gz::$url/archive/v$pkgver.tar.gz")
sha256sums=('SKIP')

build() {
    cmake -B build -S "$pkgname-$pkgver" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -Wno-dev
    cmake --build build --parallel
}

package() {
    install -Dm755 "build/$pkgname" "$pkgdir/usr/bin/$pkgname"
    install -Dm644 "$pkgname-$pkgver/README.md" "$pkgdir/usr/share/doc/$pkgname/README.md"
    install -Dm644 "$pkgname-$pkgver/LICENSE"   "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
