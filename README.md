# DSH → OpenOmniBot 内核集成

> 将 DeepSeek Harness (DSH) 的 Cordis 插件化 Agent 内核，通过官方 Node.js 源码编译 `libnode.so` 的方式，**原生嵌入 OpenOmniBot APK**，替换现有的 Python Agent Loop，实现不经过 proot 的原生 Android 运行。

## 项目结构

```
├── AGENTS.md              # AI 主控指令
├── PLAN.md                # 任务追踪
├── MEMORY.md              # 会话记忆
├── REVIEW-CHECKLIST.md    # 完成标准
├── docs/
│   └── research-arch.md   # 架构调研
├── scripts/
│   └── build-libnode.sh   # libnode.so 编译脚本
├── src/
│   ├── bridge/            # JNI 桥接层 (C + Kotlin)
│   │   ├── bridge.h       # JNI 头文件
│   │   ├── bridge.c       # JNI 实现
│   │   ├── CMakeLists.txt # NDK 构建配置
│   │   └── JniBridge.kt   # Kotlin 侧桥接
│   ├── config/
│   │   └── cordis.yml     # DSH Cordis 配置
│   └── plugins/
│       └── omnibot-adapter/  # DSH 适配插件
└── .github/workflows/
    ├── build-libnode.yml          # 编译 libnode.so
    ├── build-adapter-plugin.yml   # 编译适配插件
    └── lint-check.yml             # 代码质量检查
```

## 架构

```
┌─────────────────────────────────────────┐
│  OpenOmniBot APK                        │
│  ┌───────────────────────────────────┐  │
│  │  Android Kotlin 层                │  │
│  │  ├─ OmniFlow.kt (调度器)          │  │
│  │  └─ DshBridge.kt (JNI 桥接)      │──┼──→ 原生工具
│  ├───────────────────────────────────┤  │
│  │  libnode.so (官方自编译)           │  │  ← 原生进程
│  │  ├─ DSH Cordis 内核               │  │
│  │  └─ @omnibot/adapter 插件         │──┼──→ JNI → Kotlin
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

## CI/CD 状态

| 工作流 | 状态 | 说明 |
|:-------|:----|:-----|
| **Build libnode.so** | ⬜ 待运行 | 通过 Android NDK 编译 libnode.so (ARM64) |
| **Build @omnibot/adapter** | ⬜ 待运行 | 编译 TypeScript 适配插件 |
| **Code Quality** | ⬜ 待运行 | Markdown/YAML/C 代码格式检查 |

## 快速开始

```bash
# 1. 编译 libnode.so (需要 Android NDK)
./scripts/build-libnode.sh arm64

# 2. 编译适配插件
cd src/plugins/omnibot-adapter
npm install && npm run build

# 3. 集成到 OpenOmniBot
# 将 libnode.so 放入 app/src/main/jniLibs/arm64-v8a/
```

## 调研结论

经过全量互联网调研，Node.js 嵌入 Android 的唯二可行方案：

| 方案 | 推荐度 | 说明 |
|:-----|:------|:------|
| 官方 Node.js 源码自编译 libnode.so | 🥇 **首选** | 版本最新 (v22 LTS)，可裁剪体积 |
| nodejs-mobile 社区 fork | 🥈 备选 | 版本略落后 (v18-20) |
| QuickJS / Hermes / Bun / Deno | ❌ | 不兼容 Node.js API |

## 许可证

Apache 2.0