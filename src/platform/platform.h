#pragma once

#include <functional>
#include <string>

namespace polarclock {

/**
 * @brief Abstract base class for platform-specific initialization and main loop.
 *
 * This abstraction allows the same application code to run on different platforms
 * (Desktop via GLFW, Web via Emscripten, Android via Native Activity) without
 * macro-heavy conditional compilation in the main application code.
 */
class Platform {
public:
    virtual ~Platform() = default;

    /**
     * @brief Initialize the platform, create window/surface, and set up OpenGL context.
     * @param width Initial window width
     * @param height Initial window height
     * @param title Window title (ignored on some platforms)
     * @return true if initialization succeeded
     */
    virtual bool init(int width, int height, const char* title) = 0;

    /**
     * @brief Clean up platform resources.
     */
    virtual void shutdown() = 0;

    /**
     * @brief Run the main loop with the given frame callback.
     *
     * This method handles platform-specific main loop requirements.
     * On desktop, it runs a blocking while loop.
     * On Emscripten, it uses emscripten_set_main_loop.
     * On Android, it integrates with the app lifecycle.
     *
     * @param frameCallback Function called each frame with delta time in seconds
     */
    virtual void runMainLoop(std::function<void(float deltaTime)> frameCallback) = 0;

    /**
     * @brief Get current framebuffer dimensions.
     * @param width Output width
     * @param height Output height
     */
    virtual void getFramebufferSize(int& width, int& height) = 0;

    /**
     * @brief Swap buffers after rendering.
     */
    virtual void swapBuffers() = 0;

    /**
     * @brief Poll for input events.
     */
    virtual void pollEvents() = 0;

    /**
     * @brief Check if the application should close.
     * @return true if window close was requested
     */
    virtual bool shouldClose() = 0;

    /**
     * @brief Get the platform name for logging.
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Create the appropriate platform for the current build target.
     *
     * This factory method returns the correct Platform subclass based on
     * compile-time platform detection.
     */
    static Platform* create();
};

} // namespace polarclock
