/**
 * bridge.c — JNI 桥接层 C 实现
 *
 * 实现 bridge.h 中声明的 JNI 函数。
 * 负责初始化 Node.js 事件循环，加载 DSH Cordis 内核，
 * 并在工具调用时转发到 Kotlin 侧。
 *
 * 使用 Android NDK 的 CMake 编译，产物为 libomnibot-bridge.so。
 * 在运行时动态链接 libnode.so。
 */

#include "bridge.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* ================================================================
 *  全局状态
 * ================================================================ */

/* Node.js 平台实例 */
static node::MultiIsolatePlatform *g_platform = NULL;
static uv_loop_t *g_loop = NULL;
static pthread_t g_loop_thread;
static volatile bool g_running = false;

/* JVM 引用 — 用于回调 */
static JavaVM *g_jvm = NULL;
static jobject g_bridge_obj = NULL;
static jmethodID g_on_llm_response = NULL;
static jmethodID g_on_tool_result = NULL;

/* ================================================================
 *  辅助函数
 * ================================================================ */

static JNIEnv *get_env(void) {
    JNIEnv *env;
    (*g_jvm)->GetEnv(g_jvm, (void **)&env, JNI_VERSION_1_6);
    return env;
}

/* ================================================================
 *  事件循环线程
 * ================================================================ */

static void *event_loop_thread(void *arg) {
    (void)arg;

    /* 在此处初始化 Node.js 环境并加载 DSH Cordis 内核 */
    /* 这需要调用 Node.js 的 node::Start() 或自定义初始化 */

    g_running = true;

    /* 运行 uv 事件循环 */
    while (g_running) {
        uv_run(g_loop, UV_RUN_NOWAIT);
    }

    return NULL;
}

/* ================================================================
 *  JNI 实现
 * ================================================================ */

JNIEXPORT jint JNICALL
Java_cn_com_omnimind_bot_omniflow_DshBridge_nativeInitNodeRuntime(
    JNIEnv *env, jobject thiz,
    jstring nodeHome,
    jstring dshCorePath)
{
    /* 保存 JVM 引用用于回调 */
    (*env)->GetJavaVM(env, &g_jvm);
    g_bridge_obj = (*env)->NewGlobalRef(env, thiz);

    /* 获取方法 ID */
    jclass clazz = (*env)->GetObjectClass(env, thiz);
    g_on_llm_response = (*env)->GetMethodID(env, clazz, "onLLMResponse", "(Ljava/lang/String;)V");
    g_on_tool_result = (*env)->GetMethodID(env, clazz, "onToolResult", "(Ljava/lang/String;)V");

    /* 获取路径字符串 */
    const char *node_home = (*env)->GetStringUTFChars(env, nodeHome, NULL);
    const char *dsh_core = (*env)->GetStringUTFChars(env, dshCorePath, NULL);

    /* TODO: 初始化 Node.js 平台和事件循环 */
    g_loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
    uv_loop_init(g_loop);

    /* 启动事件循环线程 */
    pthread_create(&g_loop_thread, NULL, event_loop_thread, NULL);

    /* 释放字符串 */
    (*env)->ReleaseStringUTFChars(env, nodeHome, node_home);
    (*env)->ReleaseStringUTFChars(env, dshCorePath, dsh_core);

    return 0;
}

JNIEXPORT void JNICALL
Java_cn_com_omnimind_bot_omniflow_DshBridge_nativeShutdown(
    JNIEnv *env, jobject thiz)
{
    (void)env;
    (void)thiz;

    g_running = false;
    pthread_join(g_loop_thread, NULL);

    if (g_loop) {
        uv_loop_close(g_loop);
        free(g_loop);
        g_loop = NULL;
    }

    if (g_bridge_obj) {
        (*env)->DeleteGlobalRef(env, g_bridge_obj);
        g_bridge_obj = NULL;
    }
}

JNIEXPORT jstring JNICALL
Java_cn_com_omnimind_bot_omniflow_DshBridge_nativeCallTool(
    JNIEnv *env, jobject thiz,
    jstring toolName,
    jstring paramsJson)
{
    (void)thiz;

    const char *tool_name = (*env)->GetStringUTFChars(env, toolName, NULL);
    const char *params = (*env)->GetStringUTFChars(env, paramsJson, NULL);

    /* TODO: 通过 DSH 的 Cordis 工具系统执行工具调用 */
    /* 当前返回模拟结果 */
    const char *result = "{\"status\": \"ok\", \"message\": \"tool_call_forwarded\"}";

    (*env)->ReleaseStringUTFChars(env, toolName, tool_name);
    (*env)->ReleaseStringUTFChars(env, paramsJson, params);

    return (*env)->NewStringUTF(env, result);
}

JNIEXPORT jstring JNICALL
Java_cn_com_omnimind_bot_omniflow_DshBridge_nativeGetTools(
    JNIEnv *env, jobject thiz)
{
    (void)thiz;

    /* TODO: 从 DSH 的 Cordis 工具注册表中获取工具列表 */
    const char *tools = "[]";
    return (*env)->NewStringUTF(env, tools);
}

/* ================================================================
 *  回调函数 (从 Node.js 侧调用)
 * ================================================================ */

void omniBridge_onLLMResponse(JNIEnv *env, jobject thiz, const char *turnJson) {
    if (g_on_llm_response == NULL) return;
    jstring json = (*env)->NewStringUTF(env, turnJson);
    (*env)->CallVoidMethod(env, thiz, g_on_llm_response, json);
    (*env)->DeleteLocalRef(env, json);
}

void omniBridge_onToolResult(JNIEnv *env, jobject thiz, const char *resultJson) {
    if (g_on_tool_result == NULL) return;
    jstring json = (*env)->NewStringUTF(env, resultJson);
    (*env)->CallVoidMethod(env, thiz, g_on_tool_result, json);
    (*env)->DeleteLocalRef(env, json);
}

/* ================================================================
 *  JNI_OnLoad — 库加载时自动调用
 * ================================================================ */

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    (void)reserved;
    g_jvm = vm;
    return JNI_VERSION_1_6;
}