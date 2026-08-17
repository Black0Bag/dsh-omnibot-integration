# DSH → OpenOmniBot 内核替换 AGENTS.md

## 项目概述

将 DeepSeek Harness (DSH) 的 Cordis 插件化 Agent 内核，通过官方 Node.js 源码编译 `libnode.so` 的方式，原生嵌入 OpenOmniBot APK，替换现有的 Python Agent Loop。实现不经过 proot 的纯原生 Android 运行。

## 技术栈

| 组件 | 技术 |
|:-----|:-----|
| Agent 内核 | DSH (Cordis 插件系统, TypeScript) |
| 运行时 | Node.js 官方源码编译 `libnode.so` |
| 嵌入方式 | JNI C → Kotlin 桥接 |
| 宿主 App | OpenOmniBot (Kotlin/Android) |
| 构建工具 | Android NDK r27+ / Gradle |
| 适配插件 | `@omnibot/adapter` (TypeScript) |

## 项目结构

```
dsh-omnibot-integration/
├── PLAN.md                  # 任务追踪 (主文档)
├── AGENTS.md                # 本文件 — AI 主控指令
├── MEMORY.md                # 会话记忆
├── REVIEW-CHECKLIST.md      # 完成标准
├── docs/                    # 设计文档
│   ├── research-arch.md
│   ├── PRD-project.md
│   └── TechDesign-project.md
├── agent_docs/              # 详细技术文档
├── src/
│   ├── bridge/              # JNI 桥接层
│   ├── plugins/             # DSH 适配插件
│   └── config/              # 配置文件
└── scripts/                 # 构建脚本
```

## 工作模式

### 如何思考
- 先读 PLAN.md 确认当前阶段和任务状态
- 每个任务开始前先确认前提条件已满足
- 遇到问题时先搜索互联网寻找最佳实践
- 记录每次失败/修正到 MEMORY.md 供后续参考

### 不要做的事
- 不要跳过 PLAN.md 中的依赖步骤
- 不要修改 OpenOmniBot 源码中与 DSH 无关的模块
- 不要使用破坏性命令（rm -rf / 等）除非用户明确授权
- 不要假设 Android NDK 已安装，先检查环境

### 工程约束
- 所有代码改动必须可追溯（git commit）
- 每次里程碑完成后更新 PLAN.md 状态
- 关键配置项必须有注释说明
- 产物文件必须包含版本号

## 路线图

### Phase 1: 基础构建 (M1)
- 编译 libnode.so (Node.js v22 LTS ARM64)
- 编写 JNI 桥接层 (C + Kotlin)
- 验证 Node.js 在 Android 进程内可运行

### Phase 2: 插件适配 (M2)
- 创建 `@omnibot/adapter` 插件
- 实现 5 个工具适配器
- 编写 cordis.yml 配置

### Phase 3: 集成替换 (M3)
- 替换 OmniFlow 中的 Python Agent Loop
- 接入 DSH Cordis Agent Loop
- 适配 UI 层

### Phase 4: 打包验证 (M4)
- 编写构建脚本
- 完整编译 APK
- 功能验证 + 性能测试

## 当前状态

**当前阶段**: Phase 1 (M0 已完成)
**下一步**: 安装 Android NDK 并编译 libnode.so
**最新决策**: 采用官方 Node.js 源码自编译，放弃 nodejs-mobile (版本落后)

## 设置与命令

```bash
# 检查 NDK
echo $ANDROID_NDK_HOME

# 编译 libnode.so
cd /path/to/node
./android-configure $ANDROID_NDK_HOME arm64 --shared
make -j$(nproc)

# 查看产物
ls -lh out/Release/lib.target/libnode.so
```

## 关键决策记录

| 决策 | 选项 | 选择 | 理由 |
|:-----|:-----|:-----|:-----|
| Node.js 运行时 | nodejs-mobile / 官方自编译 | 官方自编译 | 版本最新，可控 |
| 桥接方式 | HTTP / stdin / JNI | JNI | 最低延迟 |
| 嵌入方式 | proot / 原生 | 原生 libnode.so | 不经过容器 |
| LLM 调用 | 原生 Kotlin / DSH llm 插件 | DSH llm 插件 | 统一生态 |