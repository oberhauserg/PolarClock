#include "android_platform.h"

#ifdef __ANDROID__

#include <android_native_app_glue.h>
#include "../asset_loader.h"

namespace polarclock {

AndroidPlatform::AndroidPlatform() = default;

AndroidPlatform::~AndroidPlatform() {
    shutdown();
}

bool AndroidPlatform::init(int width, int height, const char* /* title */) {
    // On Android, we can't create a window ourselves - it's provided by the system
    // The actual EGL initialization happens in setNativeWindow() when the window is ready
    m_width = width;
    m_height = height;
    m_running = true;
    m_initialized = true;
    LOGI("AndroidPlatform initialized (waiting for native window)");
    return true;
}

void AndroidPlatform::setApp(struct android_app* app) {
    m_app = app;
    if (app && app->activity) {
        setAssetManager(app->activity->assetManager);
    }
}

void AndroidPlatform::setAssetManager(AAssetManager* assetManager) {
    m_assetManager = assetManager;
    AssetLoader::instance().setAssetManager(assetManager);
    LOGI("AssetManager configured");
}

void AndroidPlatform::setNativeWindow(ANativeWindow* window) {
    if (m_window == window) return;

    // Clean up old surface if we're switching windows
    if (m_surface != EGL_NO_SURFACE) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(m_display, m_surface);
        m_surface = EGL_NO_SURFACE;
    }

    m_window = window;

    if (window) {
        if (m_display == EGL_NO_DISPLAY) {
            if (!initEGL()) {
                LOGE("Failed to initialize EGL");
                return;
            }
        }

        // Create window surface
        m_surface = eglCreateWindowSurface(m_display, m_config, window, nullptr);
        if (m_surface == EGL_NO_SURFACE) {
            LOGE("Failed to create EGL window surface: 0x%x", eglGetError());
            return;
        }

        // Make context current
        if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context)) {
            LOGE("Failed to make EGL context current: 0x%x", eglGetError());
            return;
        }

        // Query actual surface size
        eglQuerySurface(m_display, m_surface, EGL_WIDTH, &m_width);
        eglQuerySurface(m_display, m_surface, EGL_HEIGHT, &m_height);

        LOGI("EGL surface created: %dx%d", m_width, m_height);
        LOGI("OpenGL ES: %s", glGetString(GL_VERSION));
        LOGI("Renderer: %s", glGetString(GL_RENDERER));

        m_lastTime = std::chrono::high_resolution_clock::now();
    }
}

bool AndroidPlatform::initEGL() {
    // Get default display
    m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_display == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed");
        return false;
    }

    // Initialize EGL
    EGLint major, minor;
    if (!eglInitialize(m_display, &major, &minor)) {
        LOGE("eglInitialize failed: 0x%x", eglGetError());
        return false;
    }
    LOGI("EGL initialized: %d.%d", major, minor);

    // Choose config with MSAA
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 0,       // No depth buffer needed for 2D
        EGL_STENCIL_SIZE, 0,
        EGL_SAMPLE_BUFFERS, 1,   // MSAA
        EGL_SAMPLES, 4,
        EGL_NONE
    };

    EGLint numConfigs;
    if (!eglChooseConfig(m_display, configAttribs, &m_config, 1, &numConfigs) || numConfigs == 0) {
        // Fallback without MSAA
        LOGI("MSAA not available, trying without...");
        const EGLint fallbackAttribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };
        if (!eglChooseConfig(m_display, fallbackAttribs, &m_config, 1, &numConfigs) || numConfigs == 0) {
            LOGE("eglChooseConfig failed");
            return false;
        }
    }

    // Create OpenGL ES 3.0 context
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, contextAttribs);
    if (m_context == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed: 0x%x", eglGetError());
        return false;
    }

    LOGI("EGL context created successfully");
    return true;
}

void AndroidPlatform::terminateEGL() {
    if (m_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (m_context != EGL_NO_CONTEXT) {
            eglDestroyContext(m_display, m_context);
            m_context = EGL_NO_CONTEXT;
        }
        if (m_surface != EGL_NO_SURFACE) {
            eglDestroySurface(m_display, m_surface);
            m_surface = EGL_NO_SURFACE;
        }
        eglTerminate(m_display);
        m_display = EGL_NO_DISPLAY;
    }
}

void AndroidPlatform::shutdown() {
    terminateEGL();
    m_window = nullptr;
    m_running = false;
    m_initialized = false;
}

void AndroidPlatform::onPause() {
    LOGI("onPause");
    m_paused = true;
    // Release the surface but keep the context
    if (m_surface != EGL_NO_SURFACE && m_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(m_display, m_surface);
        m_surface = EGL_NO_SURFACE;
    }
}

void AndroidPlatform::onResume() {
    LOGI("onResume");
    m_paused = false;
    // Surface will be recreated when setNativeWindow is called
}

void AndroidPlatform::onDestroy() {
    LOGI("onDestroy");
    m_running = false;
}

void AndroidPlatform::runMainLoop(std::function<void(float deltaTime)> /* frameCallback */) {
    // On Android, the main loop is driven by android_main.cpp
    // This method is not used - the loop is in android_main()
    LOGI("runMainLoop called - note: Android uses external main loop");
}

void AndroidPlatform::getFramebufferSize(int& width, int& height) {
    if (m_surface != EGL_NO_SURFACE) {
        eglQuerySurface(m_display, m_surface, EGL_WIDTH, &m_width);
        eglQuerySurface(m_display, m_surface, EGL_HEIGHT, &m_height);
    }
    width = m_width;
    height = m_height;
}

void AndroidPlatform::swapBuffers() {
    if (m_surface != EGL_NO_SURFACE && m_display != EGL_NO_DISPLAY) {
        eglSwapBuffers(m_display, m_surface);
    }
}

void AndroidPlatform::pollEvents() {
    // Event polling is handled by android_native_app_glue in android_main()
}

bool AndroidPlatform::shouldClose() {
    return !m_running;
}

} // namespace polarclock

#endif // __ANDROID__
