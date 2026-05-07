# Maintainer: Nanmon <your@email.com>
pkgname=nvidia-arch-setup
pkgver=1.0.0
pkgrel=1
pkgdesc='Automatic NVIDIA driver configurator for Arch Linux'
arch=('x86_64')
url='https://github.com/USERNAME/nvidia-arch-setup'
license=('MIT')
makedepends=('cmake' 'gcc')
depends=('pciutils')
optdepends=(
    'paru: AUR helper required for Pascal/Kepler/Fermi legacy drivers'
    'yay: AUR helper required for Pascal/Kepler/Fermi legacy drivers'
)
# For local build: source=("$pkgname::git+file://$PWD")
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
}
