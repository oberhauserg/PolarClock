#pragma once

#include "platform.h"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include <chrono>

namespace polarclock {

/**
 * @brief Emscripten/WebAssembly platform implementation.
 *
 * Uses GLFW for windowing (compiled to WebGL) and Emscripten's main loop.
 * OpenGL ES 3.0 maps directly to WebGL2.
 */
class EmscriptenPlatform : public Platform {
public:
    EmscriptenPlatform();
    ~EmscriptenPlatform() override;

    bool init(int width, int height, const char* title) override;
    void shutdown() override;
    void runMainLoop(std::function<void(float deltaTime)> frameCallback) override;
    void getFramebufferSize(int& width, int& height) override;
    void swapBuffers() override;
    void pollEvents() override;
    bool shouldClose() override;
    const char* getName() const override { return "Emscripten (WebGL2)"; }

    // Static callback for emscripten_set_main_loop
    static void mainLoopCallback();

private:
    GLFWwindow* m_window = nullptr;
    std::chrono::high_resolution_clock::time_point m_lastTime;
    bool m_firstFrame = true;
    int m_width = 0;
    int m_height = 0;

    // Static state for Emscripten callback (emscripten_set_main_loop doesn't support user data)
    static EmscriptenPlatform* s_instance;
    static std::function<void(float)> s_frameCallback;
};

} // namespace polarclock

#endif // __EMSCRIPTEN__
