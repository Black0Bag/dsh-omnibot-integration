# MEMORY.md — DSH → OpenOmniBot 集成项目

## 🏗️ 当前阶段与目标

**当前阶段**: Phase 1 — 编译 libnode.so + JNI 桥接层 (M1)
**目标**: 构建可在 Android 原生进程内运行 DSH 内核的基础设施
**前置条件**: M0 已完成（架构调研和方案确认）

## 关键决策记录

- **运行时选择**: 官方 Node.js 源码自编译 libnode.so (v22 LTS)，放弃 nodejs-mobile（版本落后）
- **桥接方式**: JNI C → Kotlin，放弃 HTTP/stdin（延迟高）
- **嵌入方式**: 原生 libnode.so 嵌入 APK，不经过 proot
- **LLM 调用**: 复用 DSH 的 `@deepseek-ai/dsh-llm` 插件，不另起炉灶

## 已知问题

- Node.js v22+ 的 `android-configure` 脚本可能存在 NDK r27+ 兼容性问题 (Issue #34115)
- 备选方案：降级到 Node.js v20 LTS 或手动打补丁
- libnode.so 体积约 25MB，需裁剪 ICU/crypto 等模块

## 学习记录

- DSH 的 Cordis 内核是纯 TypeScript，通过 `nodejs-mobile` 或 libnode.so 均可运行
- OpenOmniBot 的 Agent Loop 实际运行在 Python 进程中（`OmniFlowPythonClient`），Kotlin 层是调度器
- 两者的工具调用接口高度相似，映射成本低
- 社区已有 `dsh-mobile-gui-agent` 插件证明了 ADB 控制 Android 的技术可行性