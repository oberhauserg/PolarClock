#pragma once

#include <string>

#ifdef __EMSCRIPTEN__

// Emscripten uses virtual filesystem with absolute paths
inline std::string getResourcePath(const std::string& relativePath) {
    return "/" + relativePath;
}

#else

#ifdef __linux__
#include <unistd.h>
#include <linux/limits.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

inline std::string getExecutableDir() {
    static std::string cachedDir;
    if (!cachedDir.empty()) return cachedDir;

#ifdef __linux__
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        cachedDir = std::string(path);
    }
#elif defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        cachedDir = std::string(path);
    }
#elif defined(_WIN32)
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    cachedDir = std::string(path);
#endif

    // Remove executable name to get directory
    size_t lastSlash = cachedDir.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        cachedDir = cachedDir.substr(0, lastSlash + 1);
    }
    return cachedDir;
}

inline std::string getResourcePath(const std::string& relativePath) {
    return getExecutableDir() + relativePath;
}

#endif
