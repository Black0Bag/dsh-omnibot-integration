/**
 * bridge.h — JNI 桥接层头文件
 *
 * 定义 Node.js (libnode.so) 与 Android Kotlin 层之间的 JNI 接口。
 * C 侧运行 Node.js 事件循环并加载 DSH Cordis 内核，
 * Kotlin 侧通过 JNI 调用初始化/工具转发/关闭等操作。
 *
 * 编译方式：通过 Android NDK 的 CMake 集成到 OpenOmniBot 的 Gradle 构建中。
 */

#ifndef OMNIBOT_DSH_BRIDGE_H
#define OMNIBOT_DSH_BRIDGE_H

#include <jni.h>
#include <node.h>
#include <uv.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 *  生命周期管理
 * ================================================================ */

/**
 * 初始化 Node.js 运行时。
 * 启动 Node.js 事件循环，加载 DSH 的 Cordis 内核。
 *
 * @param env        JNI 环境指针
 * @param thiz       调用此方法的 Java 对象
 * @param nodeHome    Node.js 资源目录路径 (APK assets 解压目录)
 * @param dshCorePath DSH Cordis 内核入口 JS 文件路径
 * @return 0 成功，非0 失败
 */
JNIEXPORT jint JNICALL
Java_cn_com_omnimind_bot_omniflow_DshBridge_nativeInitNodeRuntime(
    JNIEnv *env, jobject thiz,
    jstring nodeHome,
    jstring dshCorePath);

/**
 * 安全关闭 Node.js 运行时。
 * 停止事件循环，释放所有资源。
 */
JNIEXPORT void JNICALL
Java_cn_com_omnimind_bot_omniflow_DshBridge_nativeShutdown(
    JNIEnv *env, jobject thiz);

/* ================================================================
 *  工具调用转发
 * ================================================================ */

/**
 * 调用 DSH 工具。
 * 将 Kotlin 侧的工具调用请求转发到 DSH 的 Cordis 工具插件系统。
 *
 * @param env       JNI 环境指针
 * @param thiz      调用此方法的 Java 对象
 * @param toolName  工具名称 (如 "vlm_screenshot", "browser_open")
 * @param paramsJson 工具参数 (JSON 字符串)
 * @return 工具执行结果 (JSON 字符串)
 */
JNIEXPORT jstring JNICALL
Java_cn_com_omnimind_bot_omniflow_DshBridge_nativeCallTool(
    JNIEnv *env, jobject thiz,
    jstring toolName,
    jstring paramsJson);

/**
 * 获取 DSH 当前可用的工具列表。
 * 用于 Kotlin 侧注册到 LLM 的 function calling 接口。
 *
 * @return 工具列表 JSON 数组字符串
 */
JNIEXPORT jstring JNICALL
Java_cn_com_omnimind_bot_omniflow_DshBridge_nativeGetTools(
    JNIEnv *env, jobject thiz);

/* ================================================================
 *  LLM 调用回调
 * ================================================================ */

/**
 * 将 LLM 的 stream 响应转发到 Kotlin 侧。
 * DSH 的 LLM 插件完成一次推理后，通过此回调将结果送回 Kotlin。
 *
 * @param env       JNI 环境指针
 * @param thiz      调用此方法的 Java 对象
 * @param turnJson  LLM 响应 (ChatCompletionTurn JSON 字符串)
 */
void omniBridge_onLLMResponse(JNIEnv *env, jobject thiz, const char *turnJson);

/**
 * 将工具执行结果转发到 Kotlin 侧。
 * DSH 的 Agent Loop 完成一次工具调用后，通过此回调将结果送回 Kotlin。
 *
 * @param env       JNI 环境指针
 * @param thiz      调用此方法的 Java 对象
 * @param resultJson 工具执行结果 JSON 字符串
 */
void omniBridge_onToolResult(JNIEnv *env, jobject thiz, const char *resultJson);

/* ================================================================
 *  内部辅助函数
 * ================================================================ */

/**
 * 初始化 Node.js 的 uv 事件循环。
 * 在单独的线程中运行，确保不阻塞 Android 主线程。
 */
int omniBridge_startEventLoop(const char *nodeHome, const char *dshCorePath);

/**
 * 停止事件循环。
 */
void omniBridge_stopEventLoop(void);

/**
 * 检查运行时是否正在运行。
 */
bool omniBridge_isRunning(void);

#ifdef __cplusplus
}
#endif

#endif /* OMNIBOT_DSH_BRIDGE_H */