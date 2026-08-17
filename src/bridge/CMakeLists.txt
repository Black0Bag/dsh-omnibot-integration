# CMakeLists.txt — 编译 libomnibot-bridge.so
#
# 将此文件放置到 OpenOmniBot 的 app/src/main/cpp/ 目录下，
# 并在 app/build.gradle.kts 中配置 externalNativeBuild。
#
# 依赖：
#   - libnode.so (预编译，放置在 app/src/main/jniLibs/arm64-v8a/)
#   - Android NDK r27+

cmake_minimum_required(VERSION 3.22)
project("omnibot-bridge")

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 查找 libnode.so
# 预编译的 libnode.so 应放在 app/src/main/jniLibs/arm64-v8a/
set(LIBNODE_DIR ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

# 包含头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}/src/main/cpp
    ${CMAKE_SOURCE_DIR}/../../src/node-source/include  # Node.js 头文件
    ${CMAKE_SOURCE_DIR}/../../src/node-source/deps/uv/include
    ${CMAKE_SOURCE_DIR}/../../src/node-source/deps/v8/include
)

# 源文件
add_library(
    omnibot-bridge
    SHARED
    src/main/cpp/bridge.c
)

# 链接 libnode.so
target_link_libraries(
    omnibot-bridge
    log                     # Android logcat
    node                    # libnode.so
)

# 指定 libnode.so 的搜索路径
target_link_directories(
    omnibot-bridge
    PRIVATE
    ${LIBNODE_DIR}
)