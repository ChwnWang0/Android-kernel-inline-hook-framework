#!/bin/bash
###
 # @Author: 野生指针 <1703214369@qq.com>
 # @Date: 2026-04-17 19:08:11
 # @LastEditors: 野生指针 <1703214369@qq.com>
 # @LastEditTime: 2026-04-22 00:09:35
 # @FilePath: /inlinehooktest/ddk_build.sh
 # @Description: 联系我QQ 1703214369
 # 
 # Copyright (c) 2026 by 野生指针 <1703214369@qq.com>, All Rights Reserved. 
### 
set -euo pipefail

BUILD_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
DRIVER_SRC="$BUILD_ROOT/"
DOCKER_IMAGE_PREFIX="ghcr.io/ylarod/ddk"

declare -A CLANG_MAP=(
    ["android16-6.12"]="/opt/ddk/clang/clang-r536225/bin"
    ["android15-6.6"]="/opt/ddk/clang/clang-r510928/bin"
    ["android14-6.1"]="/opt/ddk/clang/clang-r487747c/bin"
    ["android13-5.15"]="/opt/ddk/clang/clang-r450784e/bin"
    ["android12-5.10"]="/opt/ddk/clang/clang-r416183b/bin"
)

usage() {
    echo "用法: $0 <version>"
    echo "可用版本: ${!CLANG_MAP[*]}"
    exit 1
}

[[ $# -lt 1 ]] && usage

VERSION="$1"
CLANG_PATH="${CLANG_MAP[$VERSION]:-}"
[[ -z "$CLANG_PATH" ]] && { echo "❌ 未知版本: $VERSION"; usage; }

echo "🚀 编译 $VERSION ..."
sudo docker run --rm \
    -v "$DRIVER_SRC":/mnt/driver \
    "$DOCKER_IMAGE_PREFIX:$VERSION" \
    bash -c "
        sed -i 's/CONFIG_DEBUG_INFO_BTF=y/CONFIG_DEBUG_INFO_BTF=n/g' /opt/ddk/kdir/$VERSION/.config 2>/dev/null || true
        export PATH=$CLANG_PATH:\$PATH
        make -C /opt/ddk/kdir/$VERSION \
            M=/mnt/driver \
            ARCH=arm64 \
            CROSS_COMPILE=aarch64-linux-gnu- \
            LLVM=1 LLVM_IAS=1 \
            CONFIG_DEBUG_INFO_BTF=n \
            modules -j\$(nproc)
    "

KO="$DRIVER_SRC/inlinehook.ko"
[[ -f "$KO" ]] && cp "$KO" "$DRIVER_SRC/${VERSION}inlinehook.ko" && echo "✅ 产物: $DRIVER_SRC/${VERSION}inlinehook.ko" || echo "❌ 编译失败"


chmod 777 -R *
