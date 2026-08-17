/**
 * @omnibot/adapter — DSH 适配插件主入口
 *
 * 将 DSH 的 Cordis 工具调用映射到 Android 原生工具。
 * 通过 JNI bridge 与 Kotlin 层的 DshBridge 通信。
 *
 * 加载方式：
 *   cordis.yml 中配置：
 *   plugins:
 *     omnibot-adapter:
 *       $include: '@omnibot/adapter'
 */

import { Context, Service } from '@deepseek-ai/cordis'
import { Tool, ToolProvider } from '@deepseek-ai/dsh-tools'
import { createLogger } from '@deepseek-ai/dsh-util'

const logger = createLogger('omnibot-adapter')

// ============================================================
// 工具定义
// ============================================================

/**
 * 工具调用接口 — 与 Kotlin 侧 JniBridge 的 nativeCallTool 对应
 */
export interface NativeToolCall {
  name: string
  params: Record<string, unknown>
}

/**
 * 工具调用结果
 */
export interface NativeToolResult {
  status: 'ok' | 'error'
  data?: unknown
  error?: string
}

// ============================================================
// JNI Bridge 接口
// ============================================================

/**
 * 通过 JNI 调用 Kotlin 侧的原生工具。
 * 在 Node.js 侧，通过注入的全局函数与 C 层通信。
 */
declare function __nativeCallTool(toolName: string, paramsJson: string): string
declare function __nativeGetTools(): string

// ============================================================
// 工具适配器实现
// ============================================================

/**
 * VLM 屏幕操作工具
 */
const vlmTool: Tool = {
  name: 'vlm_operate',
  description: '通过 VLM 模型观察和操作 Android 设备屏幕',
  parameters: {
    type: 'object',
    properties: {
      action: {
        type: 'string',
        enum: ['screenshot', 'click', 'swipe', 'type', 'back', 'home'],
        description: '要执行的操作类型',
      },
      params: {
        type: 'object',
        description: '操作参数 (坐标、文本等)',
      },
    },
    required: ['action'],
  },
  execute: async (params: Record<string, unknown>) => {
    logger.info('VLM operate:', params)
    const result = __nativeCallTool('vlm_operate', JSON.stringify(params))
    return JSON.parse(result)
  },
}

/**
 * 浏览器控制工具
 */
const browserTool: Tool = {
  name: 'browser_control',
  description: '控制离屏浏览器进行网页操作',
  parameters: {
    type: 'object',
    properties: {
      action: {
        type: 'string',
        enum: ['navigate', 'click', 'type', 'screenshot', 'get_text', 'scroll'],
        description: '浏览器操作类型',
      },
      url: { type: 'string', description: '目标 URL' },
      selector: { type: 'string', description: 'CSS 选择器' },
      text: { type: 'string', description: '输入的文本' },
    },
    required: ['action'],
  },
  execute: async (params: Record<string, unknown>) => {
    logger.info('Browser control:', params)
    const result = __nativeCallTool('browser_control', JSON.stringify(params))
    return JSON.parse(result)
  },
}

/**
 * 文件系统工具
 */
const fsTool: Tool = {
  name: 'filesystem',
  description: '读写 Android 沙箱文件系统',
  parameters: {
    type: 'object',
    properties: {
      action: {
        type: 'string',
        enum: ['read', 'write', 'list', 'delete', 'exists'],
        description: '文件操作类型',
      },
      path: { type: 'string', description: '文件路径' },
      content: { type: 'string', description: '写入内容 (write 时使用)' },
    },
    required: ['action', 'path'],
  },
  execute: async (params: Record<string, unknown>) => {
    logger.info('Filesystem:', params)
    const result = __nativeCallTool('filesystem', JSON.stringify(params))
    return JSON.parse(result)
  },
}

/**
 * 终端执行工具
 */
const terminalTool: Tool = {
  name: 'terminal_execute',
  description: '在 proot Alpine 环境中执行终端命令',
  parameters: {
    type: 'object',
    properties: {
      command: { type: 'string', description: '要执行的命令' },
      timeout: { type: 'number', description: '超时时间(秒)', default: 30 },
    },
    required: ['command'],
  },
  execute: async (params: Record<string, unknown>) => {
    logger.info('Terminal execute:', params)
    const result = __nativeCallTool('terminal_execute', JSON.stringify(params))
    return JSON.parse(result)
  },
}

/**
 * 定时器/闹钟工具
 */
const schedulerTool: Tool = {
  name: 'scheduler',
  description: '创建和管理定时任务、闹钟、日历事件',
  parameters: {
    type: 'object',
    properties: {
      action: {
        type: 'string',
        enum: ['create_alarm', 'create_calendar', 'create_timer', 'list', 'cancel'],
        description: '调度操作类型',
      },
      title: { type: 'string', description: '事件标题' },
      time: { type: 'string', description: '触发时间 (ISO-8601)' },
      repeat: { type: 'boolean', description: '是否重复', default: false },
    },
    required: ['action'],
  },
  execute: async (params: Record<string, unknown>) => {
    logger.info('Scheduler:', params)
    const result = __nativeCallTool('scheduler', JSON.stringify(params))
    return JSON.parse(result)
  },
}

// ============================================================
// 插件入口
// ============================================================

/**
 * 注册所有工具到 DSH 的 Cordis 工具系统
 */
export function apply(ctx: Context) {
  logger.info('Loading @omnibot/adapter plugin...')

  // 注册 VLM 工具
  ctx.plugin(ToolProvider, {
    tools: [vlmTool, browserTool, fsTool, terminalTool, schedulerTool],
  })

  // 注册生命周期钩子
  ctx.on('ready', () => {
    logger.info('@omnibot/adapter ready — Android native tools registered')
    logger.info('Tools: vlm_operate, browser_control, filesystem, terminal_execute, scheduler')
  })

  ctx.on('dispose', () => {
    logger.info('@omnibot/adapter disposed')
  })
}