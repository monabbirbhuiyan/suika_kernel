#!/bin/bash
# =============================================================================
# Suika OS — Cross-Compiler Toolchain Setup
# =============================================================================
# This script builds a GCC cross-compiler targeting x86_64-elf.
# Run this ONCE before building the kernel.
#
# Usage: bash meta/toolchain.sh
# =============================================================================

set -e

BINUTILS_VERSION="2.42"
GCC_VERSION="14.1.0"
TARGET="x86_64-elf"
PREFIX="$HOME/opt/cross"
SYSROOT="$HOME/opt/cross/$TARGET"

echo "=========================================="
echo "  Suika OS — Cross-Compiler Toolchain"
echo "=========================================="
echo ""
echo "Target:  $TARGET"
echo "Prefix:  $PREFIX"
echo ""

# Install prerequisites (macOS)
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "[*] Installing prerequisites via Homebrew..."
    brew install gmp mpfr mpc isl nasm qemu xorriso grub
fi

# Install prerequisites (Linux - Debian/Ubuntu)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "[*] Installing prerequisites via apt..."
    sudo apt-get update
    sudo apt-get install -y build-essential nasm qemu-system-x86 \
        xorriso grub-pc-bin grub2-common mtools gdb
fi

mkdir -p /tmp/suika_toolchain
cd /tmp/suika_toolchain

# ---------------------------------------------------------------------------
# Build Binutils
# ---------------------------------------------------------------------------
echo ""
echo "[1/3] Building Binutils $BINUTILS_VERSION..."
if [ ! -d "binutils-$BINUTILS_VERSION" ]; then
    curl -LO "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz"
    tar xf "binutils-$BINUTILS_VERSION.tar.xz"
fi

mkdir -p build-binutils
cd build-binutils
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix=$PREFIX \
    --with-sysroot --disable-nls --disable-werror --disable-shared
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
make install
cd ..

# ---------------------------------------------------------------------------
# Build GCC
# ---------------------------------------------------------------------------
echo ""
echo "[2/3] Building GCC $GCC_VERSION..."
if [ ! -d "gcc-$GCC_VERSION" ]; then
    curl -LO "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz"
    tar xf "gcc-$GCC_VERSION.tar.xz"
fi

mkdir -p build-gcc
cd build-gcc
../gcc-$GCC_VERSION/configure --target=$TARGET --prefix=$PREFIX \
    --disable-nls --enable-languages=c --without-headers \
    --with-newlib --disable-shared --disable-threads \
    --disable-libssp --disable-libgomp --disable-libmudflap \
    --disable-libquadmath --disable-libatomic
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu) all-gcc all-target-libgcc
make install-gcc install-target-libgcc
cd ..

# ---------------------------------------------------------------------------
# Done
# ---------------------------------------------------------------------------
echo ""
echo "[3/3] Toolchain installed to $PREFIX"
echo ""
echo "Add to your shell profile (~/.zshrc or ~/.bashrc):"
echo "  export PATH=\"$PREFIX/bin:\$PATH\""
echo ""
echo "Then run: source ~/.zshrc (or restart terminal)"
echo ""
echo "To verify: x86_64-elf-gcc --version"
