# 🎯 DSH → OpenOmniBot 内核替换任务追踪

> 项目目标：将 DeepSeek Harness (DSH) 的 Cordis 插件化 Agent 内核，通过官方 Node.js 源码编译 `libnode.so` 的方式，**原生嵌入 OpenOmniBot APK**，替换现有的 Python Agent Loop，实现不经过 proot 的原生 Android 运行。

## 项目结构

```
dsh-omnibot-integration/
├── PLAN.md                       # 本文件 — 任务追踪
├── AGENTS.md                     # AI 主控指令
├── MEMORY.md                     # 会话记忆
├── REVIEW-CHECKLIST.md           # 完成标准
├── docs/
│   ├── research-arch.md          # 架构调研结果
│   ├── PRD-project.md            # 产品需求
│   └── TechDesign-project.md     # 技术设计
├── agent_docs/                   # 详细文档
│   ├── tech_stack.md
│   ├── code_patterns.md
│   ├── project_brief.md
│   ├── product_requirements.md
│   └── testing.md
├── src/
│   ├── bridge/                   # JNI 桥接层
│   │   ├── bridge.h              # C 头文件定义
│   │   ├── bridge.c              # JNI 实现
│   │   └── JniBridge.kt         # Kotlin 侧桥接
│   ├── plugins/                  # DSH 适配插件
│   │   ├── omnibot-adapter/      # @omnibot/adapter 插件
│   │   │   ├── package.json
│   │   │   ├── src/index.ts
│   │   │   └── src/tools/
│   │   └── ...
│   └── config/
│       ├── cordis.yml            # DSH 配置
│       └── build-config.sh       # 构建配置
└── scripts/
    ├── build-libnode.sh          # 编译 libnode.so 脚本
    └── package-dsh.sh            # 打包 DSH 到 APK 脚本
```

---

## 📊 里程碑总览

| 里程碑 | 阶段 | 状态 | 预计工时 |
|:-------|:----|:----|:--------:|
| **M0** | 架构调研与方案确认 | ✅ 已完成 | — |
| **M1** | 编译 libnode.so + JNI 桥接 | ⬜ 待开始 | 2-3 周 |
| **M2** | DSH 适配插件开发 | ⬜ 待开始 | 1-2 周 |
| **M3** | Agent Loop 替换与集成 | ⬜ 待开始 | 1 周 |
| **M4** | 打包编译与验证 | ⬜ 待开始 | 1 周 |

---

## 🗺️ 详细任务列表

### M0：架构调研与方案确认 ✅ 已完成

- [x] 拉取 OpenOmniBot 源码并分析架构
- [x] 拉取 DSH (DeepSeek Harness) 源码并分析架构
- [x] 调研 Node.js 嵌入 Android 的所有可行方案
- [x] 确认方案：官方 Node.js 源码自编译 `libnode.so`
- [x] 确认桥接方式：JNI (C → Kotlin)
- [x] 完成接口映射表

#### 调研结论速览

| 方案 | 可行性 | 推荐度 |
|:-----|:------|:------:|
| 官方 Node.js 源码自编译 libnode.so | ✅ 可行 | 🥇 **首选** |
| nodejs-mobile 社区 fork | ⚠️ 版本落后 | 🥈 备选 |
| QuickJS / Hermes | ❌ 非 Node.js | ❌ |
| Bun / Deno | ❌ 不支持 Android | ❌ |
| Capacitor-NodeJS | ⚠️ 已 EOL | ❌ |

**构建命令**：
```bash
git clone https://github.com/nodejs/node.git
cd node
./android-configure $ANDROID_NDK_HOME arm64 --shared
make -j$(nproc)
# 产物: out/Release/lib.target/libnode.so (~25MB)
```

---

### M1：编译 libnode.so + JNI 桥接层 ⬜ 待开始

#### 1.1 编译 libnode.so

- [x] 准备 Android NDK 工具链 (r27+) — 需在 Android Studio 环境编译
- [x] 克隆 Node.js 官方源码 (v22.12.0 LTS) — 已下载到 `src/node-source/`
- [ ] 运行 `android-configure` 脚本 — 需在 NDK 环境下执行
- [ ] 执行 `make` 编译 `libnode.so` — 需在 NDK 环境下执行
- [ ] 验证产物结构 (符号表、ABI 兼容性)
- [ ] 裁剪不需要的模块减小体积

**产出**：`out/Release/lib.target/libnode.so` (ARM64)
**注意**：`android-configure` 脚本已验证存在于源码树中，实际编译需要 Android NDK 环境

#### 1.2 编写 JNI 桥接头文件 ✅ 已完成

- [x] 定义 `bridge.h`：Node.js 初始化/启动/停止接口
- [x] 定义 `bridge.h`：工具调用转发接口 (callTool)
- [x] 定义 `bridge.h`：事件回调接口 (onLLMResponse, onToolResult)

#### 1.3 编写 JNI C 实现 ✅ 已完成

- [x] 实现 `bridge.c` — JNI 函数实现 + 事件循环管理
- [x] 实现 `CMakeLists.txt` — NDK 构建配置
- [ ] 验证 NDK 编译 — 需在 Android Studio 环境执行

#### 1.4 编写 Kotlin 侧桥接 ✅ 已完成

- [x] 创建 `JniBridge.kt` — 声明 external 函数 + DshBridge object
- [x] 创建 `DshRuntime.kt` — 封装 DSH 运行时管理 (内置于 JniBridge.kt)
- [ ] 创建 `DshToolAdapter.kt` — 工具调用适配器 (延期到 M2)

**产出**：`src/bridge/` 下全部文件 (bridge.h, bridge.c, CMakeLists.txt, JniBridge.kt)

---

### M2：DSH 适配插件开发 ⬜ 待开始

#### 2.1 创建 `@omnibot/adapter` 插件 ✅ 已完成

- [x] 初始化 npm 包 `packages/omnibot-adapter/` (package.json + tsconfig.json)
- [x] 实现 Cordis 插件入口 `src/index.ts` (5 个工具注册)
- [x] 注册工具插件列表

#### 2.2 实现各工具适配器 ⬜ 待完善

| 适配器 | 文件 | 说明 |
|:-------|:-----|:-----|
| VLM 屏幕操作 | `src/tools/vlm.ts` | 截图 → JNI → Android 原生 VLM |
| 浏览器操作 | `src/tools/browser.ts` | 浏览器指令 → JNI → Android 原生浏览器 |
| 文件系统 | `src/tools/fs.ts` | 文件操作 → JNI → Android 沙箱文件系统 |
| 终端执行 | `src/tools/terminal.ts` | 命令 → JNI → proot 终端 (暂时保留) |
| 定时/日历/闹钟 | `src/tools/scheduler.ts` | 调度 → JNI → Android AlarmManager |

#### 2.3 编写 DSH 配置

- [ ] 编写 `cordis.yml` — 加载插件树
- [ ] 配置 LLM 模型接入 (复用 DSH 的 llm 插件)
- [ ] 配置工具注册

**产出**：`src/plugins/omnibot-adapter/` + `src/config/cordis.yml`

---

### M3：Agent Loop 替换与集成 ⬜ 待开始

#### 3.1 分析 OpenOmniBot 的 Agent Loop 入口

- [ ] 阅读 `OmniFlow.kt` 的 `prepareAndStart()` 方法
- [ ] 阅读 `OmniFlowPythonClient.kt` 的 Python 进程启动逻辑
- [ ] 阅读 `OmniFlowDeviceDispatcher.kt` 的工具调度逻辑

#### 3.2 替换为 DSH Runtime

- [ ] 创建 `DshRuntime.kt` — 替代 `OmniFlowPythonRuntime`
- [ ] 实现 `DshRuntime.start()` → 调用 JNI 启动 DSH Cordis 内核
- [ ] 实现 `DshRuntime.call()` → 转发到 DSH Agent Loop
- [ ] 实现 `DshRuntime.stop()` → 安全关闭

#### 3.3 适配 UI 层

- [ ] 确保 DSH 的 Web UI 与 OpenOmniBot 现有 UI 兼容
- [ ] 或复用 OpenOmniBot 的 UI，只替换底层逻辑

**产出**：替换后的 `OmniFlow.kt` 修改版

---

### M4：打包编译与验证 ⬜ 待开始

#### 4.1 构建脚本

- [ ] 编写 `build-libnode.sh` — 一键编译 libnode.so
- [ ] 编写 `package-dsh.sh` — 打包 DSH 产物到 APK assets
- [ ] 集成到 OpenOmniBot 的 Gradle 构建流程

#### 4.2 编译验证

- [ ] 完整编译 APK (需 Android Studio)
- [ ] 验证 DSH 内核正确加载
- [ ] 验证工具调用链正常
- [ ] 验证 VLM 屏幕操作
- [ ] 验证浏览器操作
- [ ] 验证文件系统操作
- [ ] 验证终端命令执行

#### 4.3 性能测试

- [ ] 测量 APK 体积增量
- [ ] 测量 DSH 内核启动时间
- [ ] 测量工具调用延迟
- [ ] 对比 Python Agent Loop 的性能差异

---

## 🔄 当前进度

**当前阶段**：M0 ✅ 已完成 → M1 ⬜ 待开始

**下一步行动**：编写 AGENTS.md + MEMORY.md + REVIEW-CHECKLIST.md，然后开始 M1。

---

## ⚠️ 已知风险

| 风险 | 概率 | 影响 | 缓解措施 |
|:-----|:----|:-----|:---------|
| Node.js v22+ android-configure 兼容性问题 | 中 | 高 | 选择 v20 LTS 作为备选 |
| libnode.so 体积过大 (25MB+) | 高 | 中 | 裁剪模块：移除 crypto/icu 等 |
| DSH 的 npm 依赖在 Android 上不兼容 | 中 | 中 | 提前梳理依赖树，用 JNI shim 替换 |
| Android 系统限制子进程 fork | 中 | 中 | 改用 DSH 的 inline 执行模式 |
| Cordis 热插拔与 Android 生命周期冲突 | 低 | 中 | 绑定到 Application.onCreate/onTerminate |

---

## 📝 修订记录

| 日期 | 版本 | 变更 |
|:----|:----|:-----|
| 2026-08-18 | v0.1 | 初始创建，基于调研结果 |