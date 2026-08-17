/**
 * JniBridge.kt — Kotlin 侧 JNI 桥接
 *
 * 声明与 C 侧 bridge.c 对应的 external 函数，
 * 封装 DshRuntime 供 OpenOmniBot 的 OmniFlow 调用。
 *
 * 包名需与 bridge.c 中 JNI 函数签名一致：
 *   cn.com.omnimind.bot.omniflow.DshBridge
 */

package cn.com.omnimind.bot.omniflow

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

/**
 * DSH 桥接器 — 管理 Node.js 运行时生命周期
 */
object DshBridge {

    private var initialized = false

    // ============================================================
    // JNI external 函数声明 (对应 bridge.c)
    // ============================================================

    private external fun nativeInitNodeRuntime(
        nodeHome: String,
        dshCorePath: String
    ): Int

    private external fun nativeShutdown()

    private external fun nativeCallTool(
        toolName: String,
        paramsJson: String
    ): String

    private external fun nativeGetTools(): String

    // ============================================================
    // 回调 — 从 C 侧调用 (在 bridge.c 中通过 JNI 调用)
    // ============================================================

    /**
     * LLM 响应回调。由 C 侧的 omniBridge_onLLMResponse() 调用。
     */
    @JvmStatic
    fun onLLMResponse(turnJson: String) {
        // TODO: 转发到 OmniFlow 的 LLM 响应处理
    }

    /**
     * 工具执行结果回调。由 C 侧的 omniBridge_onToolResult() 调用。
     */
    @JvmStatic
    fun onToolResult(resultJson: String) {
        // TODO: 转发到 OmniFlow 的工具执行处理
    }

    // ============================================================
    // 公开 API
    // ============================================================

    /**
     * 初始化 Node.js 运行时并加载 DSH 内核。
     *
     * @param context Android Context
     * @param nodeHome Node.js 资源目录 (APK assets 解压路径)
     * @param dshCorePath DSH Cordis 入口文件路径
     */
    suspend fun init(context: Context, nodeHome: String, dshCorePath: String): Result<Unit> =
        withContext(Dispatchers.IO) {
            try {
                System.loadLibrary("omnibot-bridge")
                val result = nativeInitNodeRuntime(nodeHome, dshCorePath)
                if (result == 0) {
                    initialized = true
                    Result.success(Unit)
                } else {
                    Result.failure(RuntimeException("nativeInitNodeRuntime failed: $result"))
                }
            } catch (e: Exception) {
                Result.failure(e)
            }
        }

    /**
     * 安全关闭 Node.js 运行时。
     */
    suspend fun shutdown(): Unit = withContext(Dispatchers.IO) {
        if (initialized) {
            nativeShutdown()
            initialized = false
        }
    }

    /**
     * 调用 DSH 工具。
     */
    suspend fun callTool(toolName: String, paramsJson: String): String =
        withContext(Dispatchers.IO) {
            check(initialized) { "DshBridge not initialized" }
            nativeCallTool(toolName, paramsJson)
        }

    /**
     * 获取 DSH 当前可用的工具列表。
     */
    suspend fun getTools(): String = withContext(Dispatchers.IO) {
        check(initialized) { "DshBridge not initialized" }
        nativeGetTools()
    }

    /**
     * 检查是否已初始化。
     */
    fun isInitialized(): Boolean = initialized
}

/**
 * DSH 运行时封装 — 替代 OmniFlowPythonRuntime
 *
 * 提供与现有 OmniFlow 兼容的接口，
 * 使替换对上层代码透明。
 */
class DshRuntime(private val context: Context) {

    suspend fun start(dshCorePath: String): Result<Unit> {
        val nodeHome = context.getDir("nodejs", Context.MODE_PRIVATE).absolutePath
        return DshBridge.init(context, nodeHome, dshCorePath)
    }

    suspend fun stop() {
        DshBridge.shutdown()
    }

    suspend fun call(toolName: String, params: Map<String, Any?>): String {
        val json = buildJsonString(params)
        return DshBridge.callTool(toolName, json)
    }

    private fun buildJsonString(params: Map<String, Any?>): String {
        // TODO: 使用 kotlinx.serialization 或 JSONObject
        return "{}"
    }
}