#pragma once

#include "platform.h"

#ifdef __ANDROID__

#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <chrono>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "PolarClock", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "PolarClock", __VA_ARGS__)

namespace polarclock {

/**
 * @brief Android Native Activity platform implementation.
 *
 * Uses EGL for OpenGL ES context creation and ANativeWindow for rendering surface.
 * This is a sketch showing the structure - actual implementation requires integration
 * with android_native_app_glue and proper lifecycle handling.
 */
class AndroidPlatform : public Platform {
public:
    AndroidPlatform();
    ~AndroidPlatform() override;

    bool init(int width, int height, const char* title) override;
    void shutdown() override;
    void runMainLoop(std::function<void(float deltaTime)> frameCallback) override;
    void getFramebufferSize(int& width, int& height) override;
    void swapBuffers() override;
    void pollEvents() override;
    bool shouldClose() override;
    const char* getName() const override { return "Android (EGL/GLES3)"; }

    // Called from android_main when native window is available
    void setNativeWindow(ANativeWindow* window);

    // Called from android_main on lifecycle events
    void onPause();
    void onResume();
    void onDestroy();

private:
    bool initEGL();
    void terminateEGL();

    ANativeWindow* m_window = nullptr;
    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLSurface m_surface = EGL_NO_SURFACE;
    EGLContext m_context = EGL_NO_CONTEXT;
    EGLConfig m_config = nullptr;

    int m_width = 0;
    int m_height = 0;
    bool m_running = false;
    bool m_paused = false;

    std::chrono::high_resolution_clock::time_point m_lastTime;
};

} // namespace polarclock

#endif // __ANDROID__
