#pragma once

#include "platform.h"

// Only compile on desktop platforms
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>

namespace polarclock {

/**
 * @brief Desktop platform implementation using GLFW for windowing and GLEW for OpenGL.
 *
 * Supports Linux, macOS, and Windows with OpenGL 3.3 Core/Compatibility profile.
 */
class DesktopPlatform : public Platform {
public:
    DesktopPlatform();
    ~DesktopPlatform() override;

    bool init(int width, int height, const char* title) override;
    void shutdown() override;
    void runMainLoop(std::function<void(float deltaTime)> frameCallback) override;
    void getFramebufferSize(int& width, int& height) override;
    void swapBuffers() override;
    void pollEvents() override;
    bool shouldClose() override;
    const char* getName() const override { return "Desktop (GLFW/GLEW)"; }

private:
    GLFWwindow* m_window = nullptr;
    std::chrono::high_resolution_clock::time_point m_lastTime;
};

} // namespace polarclock

#endif // Desktop only
