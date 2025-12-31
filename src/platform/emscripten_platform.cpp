#include "emscripten_platform.h"

#ifdef __EMSCRIPTEN__

namespace polarclock {

// Static members
EmscriptenPlatform* EmscriptenPlatform::s_instance = nullptr;
std::function<void(float)> EmscriptenPlatform::s_frameCallback;

EmscriptenPlatform::EmscriptenPlatform() = default;

EmscriptenPlatform::~EmscriptenPlatform() {
    shutdown();
}

bool EmscriptenPlatform::init(int width, int height, const char* title) {
    s_instance = this;

    if (!glfwInit()) {
        emscripten_log(EM_LOG_ERROR, "Failed to initialize GLFW");
        return false;
    }

    // OpenGL ES 3.0 for WebGL2
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Get canvas size from HTML
    emscripten_get_canvas_element_size("#canvas", &m_width, &m_height);
    printf("Initial window size: %d %d\n", m_width, m_height);
    if (m_width == 0 || m_height == 0) {
        m_width = width;
        m_height = height;
    }

    m_window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);
    if (!m_window) {
        emscripten_log(EM_LOG_ERROR, "Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);

    emscripten_log(EM_LOG_CONSOLE, "OpenGL context created");
    emscripten_log(EM_LOG_CONSOLE, reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    m_lastTime = std::chrono::high_resolution_clock::now();
    m_firstFrame = true;

    return true;
}

void EmscriptenPlatform::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
    s_instance = nullptr;
}

void EmscriptenPlatform::mainLoopCallback() {
    if (!s_instance || !s_frameCallback) return;

    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - s_instance->m_lastTime).count();
    s_instance->m_lastTime = currentTime;

    // On first frame, reset deltaTime and force a resize
    if (s_instance->m_firstFrame) {
        deltaTime = 0.016f;  // ~60fps
        s_instance->m_firstFrame = false;

        // Force resize on first frame to ensure correct resolution
        int w, h;
        emscripten_get_canvas_element_size("#canvas", &w, &h);
        s_instance->m_width = w;
        s_instance->m_height = h;
    }
    // Cap deltaTime to prevent animation skip after tab sleep
    else if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }

    s_frameCallback(deltaTime);
}

void EmscriptenPlatform::runMainLoop(std::function<void(float deltaTime)> frameCallback) {
    s_frameCallback = frameCallback;
    // 60 FPS, infinite loop (1 = simulate infinite loop)
    emscripten_set_main_loop(mainLoopCallback, 60, 1);
}

void EmscriptenPlatform::getFramebufferSize(int& width, int& height) {
    int newWidth, newHeight;
    emscripten_get_canvas_element_size("#canvas", &newWidth, &newHeight);

    // Update cached size if changed
    if (newWidth != m_width || newHeight != m_height) {
        m_width = newWidth;
        m_height = newHeight;
    }

    width = m_width;
    height = m_height;
}

void EmscriptenPlatform::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void EmscriptenPlatform::pollEvents() {
    glfwPollEvents();
}

bool EmscriptenPlatform::shouldClose() {
    // Web apps don't close via window close button
    return false;
}

} // namespace polarclock

#endif // __EMSCRIPTEN__
