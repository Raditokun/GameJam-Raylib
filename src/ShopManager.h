#pragma once
#include <vector>
#include "Card.h"

// ─── ShopManager ────────────────────────────────────────
// Menangani layar shop antar-wave. Shop muncul setiap
// SHOP_WAVE_INTERVAL wave. Menawarkan stok kartu tower
// T2/T3 yang berputar plus opsi "upgrade kapasitas" yang
// tetap.
//
// DEPENDENSI: ShopManager menerima const pointer ke kartu
// milik pemain (DeckManager::ownedTypes) untuk menegakkan
// aturan dependensi tier. Ia TIDAK memiliki DeckManager.

constexpr int SHOP_WAVE_INTERVAL  = 3;     // shop muncul tiap N wave
constexpr int SHOP_STOCK_SIZE     = 4;     // jumlah kartu tower acak yang ditawarkan
constexpr int CAPACITY_UPGRADE_COST = 150; // biaya menambah +1 slot tangan

struct ShopItem {
    CardDef def;            // kartu tower yang dijual
    int     price;          // biaya currency
    bool    sold;           // true setelah dibeli pada kunjungan ini
};

class DeckManager;          // deklarasi maju — tak perlu include
struct InputState;

class ShopManager {
public:
    // ── Stok ─────────────────────────────────────────────
    std::vector<ShopItem> stock;       // kartu tower T2/T3 acak
    bool capacityUpgradeSold;          // true jika sudah dibeli kunjungan ini

    // ── Status ─────────────────────────────────────────────
    bool isOpen;                       // true selama layar shop aktif

    // ── Siklus Hidup ─────────────────────────────────────────
    ShopManager();

    // Dipanggil saat shop dibuka: hasilkan stok acak
    void GenerateStock();

    // Dipanggil tiap frame selama GameState::SHOP
    // Return true saat pemain menutup shop (klik "Continue")
    bool UpdateShop(int& currency, DeckManager& deck, const InputState& in);

    void DrawShop(int currency, const DeckManager& deck, AssetManager* assets = nullptr) const;

    // ── Cek Aturan Dependensi ────────────────────────────
    // Return true jika pemain memiliki prasyarat tier lebih
    // rendah untuk suatu CardDef.
    // contoh T2 Plasma butuh memiliki T1 Plasma.
    //        T3 Plasma butuh memiliki T2 Plasma.
    //        T1 selalu return true (tanpa prasyarat).
    static bool MeetsPrerequisite(const CardDef& item, const DeckManager& deck);

    // ── Helper ───────────────────────────────────────────
    static bool ShouldOpenShop(int completedWaveIndex);  // cek apakah nomor wave memicu shop
};
