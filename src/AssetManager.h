#pragma once
#include <raylib.h>
#include <map>
#include <string>

// ─── AssetManager ───────────────────────────────────────
// Pemuatan, caching, dan pembersihan tekstur terpusat.
// Semua tekstur disimpan per key string di sebuah map.
// Panggil UnloadAll() sebelum CloseWindow() agar tidak bocor.
//
// ATURAN ORIGIN GAMBAR SPRITE:
//
// Secara default Raylib menggambar tekstur dari sudut KIRI-ATAS.
// Entitas game kita memakai posisi TENGAH. Gunakan rumus ini:
//
//   Projectile (jangkar tengah):
//     drawX = position.x - tex.width / 2
//     drawY = position.y - tex.height / 2
//
//   Tower di grid (jangkar bawah-tengah untuk stacking):
//     drawX = position.x - tex.width / 2
//     drawY = position.y - tex.height
//     Ini menjangkar bawah-tengah sprite ke node
//     grid, agar offset stacking vertikal terlihat benar.
//
//   Enemy (jangkar tengah):
//     drawX = position.x - tex.width / 2
//     drawY = position.y - tex.height / 2

class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    // Muat tekstur dari disk dan simpan dengan `key`.
    // Jika `key` sudah ada, tekstur lama di-unload dulu.
    void Load(const std::string& key, const std::string& filepath);

    // Ambil tekstur yang sudah dimuat per key. Return nullptr jika tak ada.
    Texture2D* Get(const std::string& key);

    // Return true jika key ada dan tekstur valid (id > 0).
    bool Has(const std::string& key) const;

    // Unload semua tekstur dari memori GPU dan kosongkan map.
    void UnloadAll();

    // ── Suara ───────────────────────────────────────────
    static void LoadSoundAsset(const std::string& key, const std::string& path);
    static Sound GetSound(const std::string& key);

    // ── Akses Tekstur Statis (untuk kelas tanpa instance) ─
    static Texture2D* GetTextureStatic(const std::string& key);

private:
    std::map<std::string, Texture2D> textures;
    static std::map<std::string, Sound> sounds;
    static AssetManager* instance;  // di-set oleh konstruktor
};
