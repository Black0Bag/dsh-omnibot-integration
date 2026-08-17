# 架构调研文档 — DSH → Android 原生嵌入

## 调研背景

需要将 DeepSeek Harness (DSH) 的 Agent 内核嵌入到 OpenOmniBot 中，替代现有的 Python Agent Loop，实现不经过 proot 的纯原生 Android 运行。

## 可选方案对比

### 方案 A：官方 Node.js 源码自编译 libnode.so ✅ 首选

**原理**：Node.js 官方源码树自带 `android-configure` 脚本，可交叉编译 `libnode.so` 共享库，通过 JNI 嵌入 Android App。

**步骤**：
```bash
git clone https://github.com/nodejs/node.git
cd node
export ANDROID_NDK_HOME=/path/to/ndk
./android-configure $ANDROID_NDK_HOME arm64 --shared
make -j$(nproc)
```

**产物**：`out/Release/lib.target/libnode.so` (~25MB)

**优势**：
- 版本最新 (v22 LTS / v23)
- 官方源码，可控性强
- 可裁剪体积

### 方案 B：nodejs-mobile (社区 fork)

**仓库**：`github.com/nodejs-mobile/nodejs-mobile`

**Node 版本**：v18-20

**劣势**：版本落后，原 JaneaSystems 版已不再维护。

### 方案 C：QuickJS / Hermes ❌

非 Node.js 运行时，没有 `fs`/`net`/`process` 等核心模块，无法运行 DSH。

### 方案 D：Bun / Deno ❌

不支持 Android 平台。

## 最终选择

**方案 A：官方 Node.js 源码自编译 libnode.so**

## 架构设计

```
┌─────────────────────────────────────────┐
│  OpenOmniBot APK                        │
│  ┌───────────────────────────────────┐  │
│  │  Android Kotlin 层                │  │
│  │  ├─ OmniFlow.kt (简化调度器)      │  │
│  │  ├─ OmniFlowDeviceDispatcher      │  │
│  │  └─ JniBridge.kt (新)             │──┼──→ 原生 Android 工具
│  ├───────────────────────────────────┤  │
│  │  libnode.so (官方自编译)           │  │  ← 原生进程
│  │  ├─ DSH Cordis 内核               │  │
│  │  ├─ agent-loop 插件               │  │
│  │  ├─ llm 插件                      │  │
│  │  └─ @omnibot/adapter 插件 (新)    │──┼──→ JNI → Kotlin 工具
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

## 关键接口映射

| OpenOmniBot | DSH | 桥接方式 |
|:------------|:----|:---------|
| `OmniFlowModelHost.streamTurn()` | `@deepseek-ai/dsh-llm` | 配置映射 |
| `OmniFlowDeviceDispatcher.execute()` | `@deepseek-ai/dsh-tools` | JNI 桥接 |
| `OmniFlowPlatform.startProcess()` | `@deepseek-ai/dsh-host` | JNI 桥接 |
| `OmniFlowPluginRuntime` | Cordis 插件系统 | 直接使用 |