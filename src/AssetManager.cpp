#include "AssetManager.h"
#include <cstdio>

AssetManager::AssetManager() {}

AssetManager::~AssetManager() {
    UnloadAll();
}

void AssetManager::Load(const std::string& key, const std::string& filepath) {
    // If key already exists, unload the old texture first
    auto it = textures.find(key);
    if (it != textures.end()) {
        UnloadTexture(it->second);
        textures.erase(it);
    }

    Texture2D tex = LoadTexture(filepath.c_str());
    if (tex.id == 0) {
        printf("[AssetManager] WARNING: Failed to load \"%s\" from \"%s\"\n",
               key.c_str(), filepath.c_str());
        return;
    }

    textures[key] = tex;
    printf("[AssetManager] Loaded \"%s\" (%dx%d) from \"%s\"\n",
           key.c_str(), tex.width, tex.height, filepath.c_str());
}

Texture2D* AssetManager::Get(const std::string& key) {
    auto it = textures.find(key);
    if (it == textures.end()) return nullptr;
    return &it->second;
}

bool AssetManager::Has(const std::string& key) const {
    auto it = textures.find(key);
    return it != textures.end() && it->second.id > 0;
}

void AssetManager::UnloadAll() {
    for (auto& pair : textures) {
        if (pair.second.id > 0) {
            UnloadTexture(pair.second);
            printf("[AssetManager] Unloaded \"%s\"\n", pair.first.c_str());
        }
    }
    textures.clear();
}
