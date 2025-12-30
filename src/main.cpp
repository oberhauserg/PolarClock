#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
#define LOG(msg) emscripten_log(EM_LOG_CONSOLE, "%s", msg)
#define LOG_ERR(msg) emscripten_log(EM_LOG_ERROR, "%s", msg)
#else
#include <GL/glew.h>
#define LOG(msg) std::cout << msg << std::endl
#define LOG_ERR(msg) std::cerr << msg << std::endl
#endif

#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

#include "renderer.h"
#include "polar_clock.h"
#include "theme.h"

// Global state for Emscripten main loop
struct AppState {
    GLFWwindow* window;
    polarclock::Renderer renderer;
    polarclock::PolarClock clock;
    std::chrono::high_resolution_clock::time_point lastTime;
    int width;
    int height;
};

AppState g_state;

static bool firstFrame = true;

void mainLoop() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - g_state.lastTime).count();
    g_state.lastTime = currentTime;

    // On first frame, reset deltaTime to ensure animation starts fresh
    if (firstFrame) {
        deltaTime = 0.016f;  // ~60fps
        firstFrame = false;
    }
    // Cap deltaTime to prevent animation skip after tab sleep
    else if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }

    // Check for resize
#ifdef __EMSCRIPTEN__
    int newWidth, newHeight;
    emscripten_get_canvas_element_size("#canvas", &newWidth, &newHeight);
    if (newWidth != g_state.width || newHeight != g_state.height) {
        g_state.width = newWidth;
        g_state.height = newHeight;
        g_state.renderer.resize(newWidth, newHeight);
    }
#else
    int newWidth, newHeight;
    glfwGetFramebufferSize(g_state.window, &newWidth, &newHeight);
    if (newWidth != g_state.width || newHeight != g_state.height) {
        g_state.width = newWidth;
        g_state.height = newHeight;
        g_state.renderer.resize(newWidth, newHeight);
    }
#endif

    g_state.clock.update(deltaTime);
    g_state.renderer.render(g_state.clock);

    glfwSwapBuffers(g_state.window);
    glfwPollEvents();
}

#ifdef __EMSCRIPTEN__
void emscriptenMainLoop() {
    mainLoop();
}
#endif

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version hints
#ifdef __EMSCRIPTEN__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
#endif

    glfwWindowHint(GLFW_SAMPLES, 4);  // MSAA

    g_state.width = 800;
    g_state.height = 800;

#ifdef __EMSCRIPTEN__
    // Get canvas size from HTML
    emscripten_get_canvas_element_size("#canvas", &g_state.width, &g_state.height);
#endif

    g_state.window = glfwCreateWindow(g_state.width, g_state.height, "Polar Clock", nullptr, nullptr);
    if (!g_state.window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(g_state.window);

#ifndef __EMSCRIPTEN__
    // Initialize GLEW for desktop OpenGL
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
#endif

    // Enable VSync (must be after main loop is set up on Emscripten)
#ifndef __EMSCRIPTEN__
    glfwSwapInterval(1);
#endif

    // Enable MSAA (not available in WebGL)
#ifndef __EMSCRIPTEN__
    glEnable(GL_MULTISAMPLE);
#endif

    LOG("OpenGL context created");
    LOG((const char*)glGetString(GL_VERSION));

    LOG("Initializing renderer...");
    if (!g_state.renderer.init(g_state.width, g_state.height)) {
        LOG_ERR("Failed to initialize renderer");
        return -1;
    }
    LOG("Renderer initialized successfully");

    // g_state.clock.setTheme(polarclock::createDefaultTheme());
    // g_state.renderer.setTheme(polarclock::createDefaultTheme());

    g_state.lastTime = std::chrono::high_resolution_clock::now();

    LOG("Starting main loop...");
#ifdef __EMSCRIPTEN__
    // Limit to 60fps to reduce CPU usage
    emscripten_set_main_loop(emscriptenMainLoop, 60, 1);
#else
    while (!glfwWindowShouldClose(g_state.window)) {
        mainLoop();
    }

    glfwDestroyWindow(g_state.window);
    glfwTerminate();
#endif

    return 0;
}
