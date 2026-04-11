#pragma once
#include <raylib.h>
#include <map>
#include <string>

// ─── AssetManager ───────────────────────────────────────
// Centralized texture loading, caching, and cleanup.
// All textures are stored by string key in a map.
// Call UnloadAll() before CloseWindow() to avoid leaks.
//
// SPRITE ORIGIN DRAWING RULES:
//
// Raylib draws textures from the TOP-LEFT corner by default.
// Our game entities use CENTER positions. Use these formulas:
//
//   Projectile (center-anchored):
//     drawX = position.x - tex.width / 2
//     drawY = position.y - tex.height / 2
//
//   Tower on grid (bottom-center anchored for stacking):
//     drawX = position.x - tex.width / 2
//     drawY = position.y - tex.height
//     This anchors the sprite's bottom-center to the grid
//     node, so vertical stacking offsets look correct.
//
//   Enemy (center-anchored):
//     drawX = position.x - tex.width / 2
//     drawY = position.y - tex.height / 2

class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    // Load a texture from disk and store it under `key`.
    // If `key` already exists, the old texture is unloaded first.
    void Load(const std::string& key, const std::string& filepath);

    // Retrieve a loaded texture by key. Returns nullptr if not found.
    Texture2D* Get(const std::string& key);

    // Returns true if the key exists and the texture is valid (id > 0).
    bool Has(const std::string& key) const;

    // Unload all textures from GPU memory and clear the map.
    void UnloadAll();

    // ── Sounds ───────────────────────────────────────────
    static void LoadSoundAsset(const std::string& key, const std::string& path);
    static Sound GetSound(const std::string& key);

    // ── Static Texture Access (for classes without instance) ─
    static Texture2D* GetTextureStatic(const std::string& key);

private:
    std::map<std::string, Texture2D> textures;
    static std::map<std::string, Sound> sounds;
    static AssetManager* instance;  // set by constructor
};
