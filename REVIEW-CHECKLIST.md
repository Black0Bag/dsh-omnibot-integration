# ✅ 完成标准检查清单

## 通用检查

- [ ] 所有代码改动有 git commit
- [ ] 关键路径有注释
- [ ] 没有硬编码的密钥/路径
- [ ] 与现有 OpenOmniBot 模块不冲突

## M1: libnode.so + JNI 桥接

- [ ] `libnode.so` 编译成功 (ARM64, 体积 < 30MB)
- [ ] `bridge.h` 定义了完整的接口
- [ ] `bridge.c` 通过 NDK 编译成功
- [ ] `JniBridge.kt` 声明了所有 external 函数
- [ ] 在 Android 进程内可成功初始化 Node.js 事件循环
- [ ] 可加载 DSH 的 Cordis 内核

## M2: DSH 适配插件

- [ ] `@omnibot/adapter` 插件通过 Cordis 加载
- [ ] VLM 工具适配器可正常截图和操作屏幕
- [ ] 浏览器工具适配器可正常打开网页
- [ ] 文件系统工具适配器可读写 Android 沙箱
- [ ] 终端工具适配器可执行命令
- [ ] 定时器工具适配器可设置闹钟

## M3: Agent Loop 替换

- [ ] OpenOmniBot 启动时加载 DSH 内核而非 Python 进程
- [ ] 用户输入可正常触发 DSH Agent Loop
- [ ] 工具调用链完整 (LLM → 工具选择 → 执行 → 结果返回)
- [ ] 多轮对话正常
- [ ] 停止/中断功能正常

## M4: 打包验证

- [ ] APK 编译成功
- [ ] APK 体积增量 < 40MB
- [ ] DSH 内核启动时间 < 3秒
- [ ] 所有原生工具调用延迟 < 500ms
- [ ] 基础功能回归测试全部通过