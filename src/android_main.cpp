#ifdef __ANDROID__

#include <android_native_app_glue.h>
#include <android/log.h>
#include <chrono>

#include "platform/android_platform.h"
#include "renderer.h"
#include "polar_clock.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "PolarClock", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "PolarClock", __VA_ARGS__)

namespace {
    polarclock::AndroidPlatform* g_platform = nullptr;
    polarclock::Renderer* g_renderer = nullptr;
    polarclock::PolarClock* g_clock = nullptr;
    bool g_rendererInitialized = false;
    int g_lastWidth = 0;
    int g_lastHeight = 0;
    std::chrono::high_resolution_clock::time_point g_lastTime;
    bool g_firstFrame = true;
}

static void initRenderer() {
    if (g_rendererInitialized || !g_platform->hasValidSurface()) {
        return;
    }

    LOGI("Initializing renderer...");

    g_renderer = new polarclock::Renderer();
    g_clock = new polarclock::PolarClock();

    int width, height;
    g_platform->getFramebufferSize(width, height);

    if (g_renderer->init(width, height)) {
        g_rendererInitialized = true;
        g_lastWidth = width;
        g_lastHeight = height;
        g_lastTime = std::chrono::high_resolution_clock::now();
        g_firstFrame = true;
        LOGI("Renderer initialized: %dx%d", width, height);
    } else {
        LOGE("Failed to initialize renderer");
        delete g_renderer;
        delete g_clock;
        g_renderer = nullptr;
        g_clock = nullptr;
    }
}

static void handleAppCmd(struct android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            LOGI("APP_CMD_INIT_WINDOW");
            if (app->window && g_platform) {
                g_platform->setNativeWindow(app->window);
                initRenderer();
            }
            break;

        case APP_CMD_TERM_WINDOW:
            LOGI("APP_CMD_TERM_WINDOW");
            if (g_platform) {
                g_platform->setNativeWindow(nullptr);
            }
            break;

        case APP_CMD_GAINED_FOCUS:
            LOGI("APP_CMD_GAINED_FOCUS");
            break;

        case APP_CMD_LOST_FOCUS:
            LOGI("APP_CMD_LOST_FOCUS");
            break;

        case APP_CMD_PAUSE:
            LOGI("APP_CMD_PAUSE");
            if (g_platform) {
                g_platform->onPause();
            }
            break;

        case APP_CMD_RESUME:
            LOGI("APP_CMD_RESUME");
            if (g_platform) {
                g_platform->onResume();
            }
            break;

        case APP_CMD_DESTROY:
            LOGI("APP_CMD_DESTROY");
            if (g_platform) {
                g_platform->onDestroy();
            }
            break;

        case APP_CMD_CONFIG_CHANGED:
            LOGI("APP_CMD_CONFIG_CHANGED");
            break;

        case APP_CMD_WINDOW_RESIZED:
            LOGI("APP_CMD_WINDOW_RESIZED");
            break;

        default:
            break;
    }
}

static int32_t handleInputEvent(struct android_app* /* app */, AInputEvent* event) {
    // Handle touch events if needed in the future
    int32_t eventType = AInputEvent_getType(event);
    if (eventType == AINPUT_EVENT_TYPE_MOTION) {
        // Touch event - could be used for interaction later
        return 0;  // Event not consumed
    }
    return 0;
}

static void renderFrame() {
    if (!g_rendererInitialized || !g_platform->hasValidSurface()) {
        return;
    }

    // Calculate delta time
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - g_lastTime).count();
    g_lastTime = currentTime;

    // On first frame, use a small delta time to ensure animations play
    if (g_firstFrame) {
        deltaTime = 0.016f;  // ~60fps
        g_firstFrame = false;
    } else if (deltaTime > 0.1f) {
        // Cap delta time to prevent animation skip after pause
        deltaTime = 0.1f;
    }

    // Check for resize
    int newWidth, newHeight;
    g_platform->getFramebufferSize(newWidth, newHeight);
    if (newWidth != g_lastWidth || newHeight != g_lastHeight) {
        g_lastWidth = newWidth;
        g_lastHeight = newHeight;
        g_renderer->resize(newWidth, newHeight);
        LOGI("Resized to %dx%d", newWidth, newHeight);
    }

    // Update and render
    g_clock->update(deltaTime);
    g_renderer->render(*g_clock);
    g_platform->swapBuffers();
}

void android_main(struct android_app* app) {
    LOGI("android_main started");

    // Set up callbacks
    app->onAppCmd = handleAppCmd;
    app->onInputEvent = handleInputEvent;

    // Create platform
    g_platform = new polarclock::AndroidPlatform();
    g_platform->setApp(app);
    g_platform->init(0, 0, "PolarClock");  // Size determined by window

    LOGI("Entering main loop...");

    // Main loop
    while (!app->destroyRequested) {
        // Poll events
        int events;
        struct android_poll_source* source;

        // Block while paused (timeout = -1), poll otherwise (timeout = 0)
        int timeout = (g_platform->isPaused() || !g_platform->hasValidSurface()) ? -1 : 0;

        while (ALooper_pollAll(timeout, nullptr, &events, reinterpret_cast<void**>(&source)) >= 0) {
            if (source) {
                source->process(app, source);
            }

            if (app->destroyRequested) {
                break;
            }

            // After processing events, check if we should continue blocking
            timeout = (g_platform->isPaused() || !g_platform->hasValidSurface()) ? -1 : 0;
        }

        // Render frame if not paused and surface is valid
        if (!g_platform->isPaused() && g_platform->hasValidSurface()) {
            renderFrame();
        }
    }

    // Cleanup
    LOGI("Cleaning up...");
    delete g_clock;
    g_clock = nullptr;
    delete g_renderer;
    g_renderer = nullptr;
    g_rendererInitialized = false;

    g_platform->shutdown();
    delete g_platform;
    g_platform = nullptr;

    LOGI("android_main finished");
}

#endif // __ANDROID__
