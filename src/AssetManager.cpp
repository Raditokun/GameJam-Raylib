#include "AssetManager.h"
#include <cstdio>

std::map<std::string, Sound> AssetManager::sounds;

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

    for (auto& pair : sounds) {
        if (pair.second.stream.buffer != NULL) {
            UnloadSound(pair.second);
            printf("[AssetManager] Unloaded sound \"%s\"\n", pair.first.c_str());
        }
    }
    sounds.clear();
}

void AssetManager::LoadSoundAsset(const std::string& key, const std::string& path) {
    auto it = sounds.find(key);
    if (it != sounds.end()) {
        UnloadSound(it->second);
        sounds.erase(it);
    }
    Sound snd = LoadSound(path.c_str());
    if (snd.stream.buffer == NULL) {
        printf("[AssetManager] WARNING: Failed to load sound \"%s\" from \"%s\"\n", key.c_str(), path.c_str());
        return;
    }
    sounds[key] = snd;
    printf("[AssetManager] Loaded sound \"%s\" from \"%s\"\n", key.c_str(), path.c_str());
}

Sound AssetManager::GetSound(const std::string& key) {
    auto it = sounds.find(key);
    if (it == sounds.end()) {
        Sound empty = {0};
        return empty;
    }
    return it->second;
}
