#include "platform/platform.h"
#include "renderer.h"
#include "polar_clock.h"

#include <iostream>
#include <memory>

int main() {
    // Create platform-specific implementation
    std::unique_ptr<polarclock::Platform> platform(polarclock::Platform::create());

    std::cout << "Platform: " << platform->getName() << std::endl;

    if (!platform->init(800, 800, "Polar Clock")) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return -1;
    }

    // Initialize renderer
    polarclock::Renderer renderer;
    int width, height;
    platform->getFramebufferSize(width, height);

    std::cout << "Initializing renderer..." << std::endl;
    if (!renderer.init(width, height)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }
    std::cout << "Renderer initialized successfully" << std::endl;

    // Initialize clock
    polarclock::PolarClock clock;

    // Track last known size for resize detection
    int lastWidth = width;
    int lastHeight = height;
    renderer.resize(width, height);

    std::cout << "Starting main loop..." << std::endl;

    // Run main loop with frame callback
    platform->runMainLoop([&](float deltaTime) {
        // Check for resize
        int newWidth, newHeight;
        platform->getFramebufferSize(newWidth, newHeight);
        renderer.resize(newWidth, newHeight);
        // if (newWidth != lastWidth || newHeight != lastHeight) {
        //     std::cout << "Size updated to " << newWidth << " " << newHeight << std::endl;
        //     lastWidth = newWidth;
        //     lastHeight = newHeight;
        //     renderer.resize(newWidth, newHeight);
        // }

        // Update and render
        clock.update(deltaTime);
        renderer.render(clock);

        // Swap and poll
        platform->swapBuffers();
        platform->pollEvents();
    });

    platform->shutdown();
    return 0;
}
