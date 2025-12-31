#pragma once

#include <string>
#include <vector>

#ifdef __ANDROID__
#include <android/asset_manager.h>
#endif

namespace polarclock {

/**
 * @brief Cross-platform asset loader.
 *
 * Provides a unified interface for loading assets across platforms:
 * - Desktop: Uses std::ifstream from filesystem via getResourcePath()
 * - Emscripten: Uses std::ifstream from virtual filesystem
 * - Android: Uses AAssetManager to read from APK assets
 */
class AssetLoader {
public:
    static AssetLoader& instance();

#ifdef __ANDROID__
    /**
     * @brief Set the Android asset manager (must be called before loading assets).
     */
    void setAssetManager(AAssetManager* mgr);
#endif

    /**
     * @brief Load a binary file into a byte vector.
     * @param path Relative path to the asset (e.g., "assets/font.ttf")
     * @param data Output vector to receive file contents
     * @return true if loading succeeded
     */
    bool loadFile(const std::string& path, std::vector<unsigned char>& data);

    /**
     * @brief Load a text file into a string.
     * @param path Relative path to the asset (e.g., "shaders/arc.vert")
     * @param content Output string to receive file contents
     * @return true if loading succeeded
     */
    bool loadTextFile(const std::string& path, std::string& content);

private:
    AssetLoader() = default;
    AssetLoader(const AssetLoader&) = delete;
    AssetLoader& operator=(const AssetLoader&) = delete;

#ifdef __ANDROID__
    AAssetManager* m_assetManager = nullptr;
#endif
};

} // namespace polarclock
