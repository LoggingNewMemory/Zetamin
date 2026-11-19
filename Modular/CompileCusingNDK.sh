#!/bin/bash
# ==============================================================================
# Zetamin: All in one Screen Tweaks
# Copyright (C) 2026 Kana Wesley Yamada
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
# ==============================================================================


# ==========================================
# Compile C files using Android NDK
# ==========================================

export NDK=/opt/android-ndk
export API=28
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin

SRC_DIR="Source"
if ! ls $SRC_DIR/*.c >/dev/null 2>&1; then
    echo "No C source files found in $SRC_DIR, skipping C compilation."
    exit 0
fi

echo "Compiling C files using NDK..."

for c_file in $SRC_DIR/*.c; do
    filename=$(basename -- "$c_file")
    binary_name="${filename%.*}"
    TARGET_DIR="Modules"
    mkdir -p "$TARGET_DIR"
    
    echo "Building $binary_name..."
    if ! $TOOLCHAIN/aarch64-linux-android$API-clang -Wall -O2 \
        -o "$TARGET_DIR/$binary_name" \
        "$c_file"; then
        echo "Error: Compilation of $binary_name failed!"
        exit 1
    fi
    $TOOLCHAIN/llvm-strip "$TARGET_DIR/$binary_name"
    echo "Successfully compiled $binary_name"
done
