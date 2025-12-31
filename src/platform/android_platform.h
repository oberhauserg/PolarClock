#pragma once

#include "platform.h"

#ifdef __ANDROID__

#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <chrono>
#include <functional>

// Forward declaration for android_native_app_glue
struct android_app;

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "PolarClock", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "PolarClock", __VA_ARGS__)

namespace polarclock {

/**
 * @brief Android Native Activity platform implementation.
 *
 * Uses EGL for OpenGL ES 3.0 context creation and integrates with
 * android_native_app_glue for lifecycle management.
 */
class AndroidPlatform : public Platform {
public:
    AndroidPlatform();
    ~AndroidPlatform() override;

    // Platform interface
    bool init(int width, int height, const char* title) override;
    void shutdown() override;
    void runMainLoop(std::function<void(float deltaTime)> frameCallback) override;
    void getFramebufferSize(int& width, int& height) override;
    void swapBuffers() override;
    void pollEvents() override;
    bool shouldClose() override;
    const char* getName() const override { return "Android (EGL/GLES3)"; }

    // Android-specific initialization
    void setApp(struct android_app* app);
    void setNativeWindow(ANativeWindow* window);
    void setAssetManager(AAssetManager* assetManager);

    // Lifecycle callbacks (called from android_main event handler)
    void onPause();
    void onResume();
    void onDestroy();

    // State queries
    bool isPaused() const { return m_paused; }
    bool hasValidSurface() const { return m_surface != EGL_NO_SURFACE; }
    bool isInitialized() const { return m_initialized; }

private:
    bool initEGL();
    void terminateEGL();

    struct android_app* m_app = nullptr;
    ANativeWindow* m_window = nullptr;
    AAssetManager* m_assetManager = nullptr;

    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLSurface m_surface = EGL_NO_SURFACE;
    EGLContext m_context = EGL_NO_CONTEXT;
    EGLConfig m_config = nullptr;

    int m_width = 0;
    int m_height = 0;
    bool m_running = false;
    bool m_paused = false;
    bool m_initialized = false;

    std::chrono::high_resolution_clock::time_point m_lastTime;
};

} // namespace polarclock

#endif // __ANDROID__
