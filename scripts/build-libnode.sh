#!/bin/bash
# ============================================================
# build-libnode.sh — 编译 Node.js 共享库 libnode.so for Android
#
# 前置条件：
#   1. 安装 Android NDK r27+ (设置 $ANDROID_NDK_HOME)
#   2. 已安装 Python 3.x
#   3. 已安装 make, ccache (可选)
#
# 用法：
#   export ANDROID_NDK_HOME=/path/to/ndk
#   ./build-libnode.sh [arm64|arm|x86_64]
#
# 产物：
#   out/Release/lib.target/libnode.so
# ============================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
NODE_SOURCE_DIR="$PROJECT_DIR/src/node-source"
OUTPUT_DIR="$PROJECT_DIR/out"

# 默认目标架构
ARCH="${1:-arm64}"

# 架构映射
case "$ARCH" in
    arm64|aarch64)
        DEST_CPU="arm64"
        DEST_OS="android"
        ;;
    arm)
        DEST_CPU="arm"
        DEST_OS="android"
        ;;
    x86_64|amd64)
        DEST_CPU="x64"
        DEST_OS="android"
        ;;
    *)
        echo "❌ 不支持的架构: $ARCH (可选: arm64, arm, x86_64)"
        exit 1
        ;;
esac

echo "=============================================="
echo " 编译 Node.js libnode.so for Android"
echo " 架构:       $DEST_CPU"
echo " NDK:        ${ANDROID_NDK_HOME:-未设置}"
echo " Node 源码:  $NODE_SOURCE_DIR"
echo "=============================================="

# 检查前置条件
if [ -z "${ANDROID_NDK_HOME:-}" ]; then
    echo "❌ 错误: 未设置 ANDROID_NDK_HOME 环境变量"
    echo "   请先安装 Android NDK 并设置环境变量:"
    echo "   export ANDROID_NDK_HOME=/path/to/android-ndk-r27"
    exit 1
fi

if [ ! -d "$NODE_SOURCE_DIR" ]; then
    echo "❌ 错误: Node.js 源码目录不存在: $NODE_SOURCE_DIR"
    echo "   请先克隆 Node.js 源码:"
    echo "   git clone --depth 1 --branch v22.12.0 https://github.com/nodejs/node.git $NODE_SOURCE_DIR"
    exit 1
fi

if [ ! -f "$NODE_SOURCE_DIR/android-configure" ]; then
    echo "❌ 错误: 未找到 android-configure 脚本"
    echo "   请确认 Node.js 源码版本 ≥ v18"
    exit 1
fi

echo ""
echo "📦 步骤 1/2: 配置交叉编译环境并编译"
cd "$NODE_SOURCE_DIR"

# android-configure 设置交叉编译环境: <NDK路径> <SDK版本> <目标架构>
# 它会设置 CC/CXX/GYP_DEFINES 并调用 ./configure --dest-cpu=... --dest-os=android
./android-configure "$ANDROID_NDK_HOME" "34" "$DEST_CPU"

# 保存 GYP_DEFINES (包含 android_ndk_path 等关键变量)
GYP_DEFINES_SAVED="${GYP_DEFINES}"

# 重新运行 ./configure 但保留 GYP_DEFINES
# 额外添加 --shared (编译为共享库 libnode.so)
# --without-npm (不编译 npm, 减少体积)
# --without-intl (不编译 ICU, 减少体积 ~20MB)
./configure \
    --shared \
    --without-npm \
    --without-intl \
    --dest-cpu="$DEST_CPU" \
    --dest-os=android \
    --openssl-no-asm \
    --cross-compiling

# 修复 GYP_DEFINES
export GYP_DEFINES="${GYP_DEFINES_SAVED}"

echo ""
echo "🔧 步骤 2/2: 开始编译 (这可能需要 10-30 分钟)"
make -j$(nproc) 2>&1 | tee build.log

echo ""
echo "✅ 步骤 3/3: 编译完成"
echo ""

# 验证产物
LIBNODE_PATH="out/Release/lib.target/libnode.so"
if [ -f "$LIBNODE_PATH" ]; then
    FILE_SIZE=$(ls -lh "$LIBNODE_PATH" | awk '{print $5}')
    echo "📦 产物: $LIBNODE_PATH"
    echo "   大小: $FILE_SIZE"
    echo "   架构: $DEST_CPU"

    # 复制到项目输出目录
    mkdir -p "$OUTPUT_DIR/$ARCH"
    cp "$LIBNODE_PATH" "$OUTPUT_DIR/$ARCH/libnode.so"
    echo "   已复制到: $OUTPUT_DIR/$ARCH/libnode.so"

    # 检查符号
    echo ""
    echo "📋 符号检查:"
    $ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-readelf \
        -h "$LIBNODE_PATH" 2>/dev/null | grep -E "Class|Machine|Entry" || true

    echo ""
    echo "🎉 编译成功!"
else
    echo "❌ 错误: 编译产物未找到!"
    exit 1
fi