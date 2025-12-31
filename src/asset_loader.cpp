#include "asset_loader.h"
#include "resource_path.h"
#include <fstream>
#include <iostream>

#ifdef __ANDROID__
#include <android/log.h>
#define ASSET_LOG(...) __android_log_print(ANDROID_LOG_INFO, "AssetLoader", __VA_ARGS__)
#define ASSET_ERR(...) __android_log_print(ANDROID_LOG_ERROR, "AssetLoader", __VA_ARGS__)
#else
#define ASSET_LOG(...) (void)0
#define ASSET_ERR(...) std::cerr << "AssetLoader: " << __VA_ARGS__ << std::endl
#endif

namespace polarclock {

AssetLoader& AssetLoader::instance() {
    static AssetLoader loader;
    return loader;
}

#ifdef __ANDROID__
void AssetLoader::setAssetManager(AAssetManager* mgr) {
    m_assetManager = mgr;
    ASSET_LOG("AssetManager configured");
}
#endif

bool AssetLoader::loadFile(const std::string& path, std::vector<unsigned char>& data) {
#ifdef __ANDROID__
    if (!m_assetManager) {
        ASSET_ERR("AssetManager not set!");
        return false;
    }

    AAsset* asset = AAssetManager_open(m_assetManager, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        ASSET_ERR("Failed to open asset: %s", path.c_str());
        return false;
    }

    off_t size = AAsset_getLength(asset);
    data.resize(static_cast<size_t>(size));
    AAsset_read(asset, data.data(), size);
    AAsset_close(asset);

    ASSET_LOG("Loaded asset: %s (%ld bytes)", path.c_str(), static_cast<long>(size));
    return true;
#else
    // Desktop and Emscripten: use filesystem
    std::string fullPath = getResourcePath(path);
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        ASSET_ERR("Failed to open file: " << fullPath);
        return false;
    }

    auto size = file.tellg();
    file.seekg(0);
    data.resize(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(data.data()), size);
    return file.good();
#endif
}

bool AssetLoader::loadTextFile(const std::string& path, std::string& content) {
    std::vector<unsigned char> data;
    if (!loadFile(path, data)) {
        return false;
    }
    content.assign(data.begin(), data.end());
    return true;
}

} // namespace polarclock
