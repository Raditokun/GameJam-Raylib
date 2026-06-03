#pragma once
#include <vector>
#include <set>
#include "Card.h"

class AssetManager;
struct InputState;

// ─── DeckManager (Dirombak) ───────────────────────────
// Kini mendukung:
//  - Kapasitas tangan dinamis (mulai 3, bisa di-upgrade ke 5)
//  - Pembatasan drafting hanya T1
//  - Pelacakan kepemilikan berbasis dependensi tier
//  - Menambah kartu dari ShopManager

class DeckManager {
public:
    // ── Pool Draft ────────────────────────────────────────
    std::vector<Card> pool;
    std::vector<int>  draftPicks;       // indeks ke pool
    bool draftComplete;

    // ── Tangan Aktif (PLAYING) ────────────────────────────
    std::vector<Card> hand;
    int selectedHandIndex;

    // ── Sistem Kapasitas ──────────────────────────────────
    int   maxHandSize;                  // mulai 3, bisa di-upgrade ke MAX_HAND_CAP
    static constexpr int STARTING_HAND_CAP = 3;
    static constexpr int MAX_HAND_CAP      = 5;

    // ── Pelacakan Kepemilikan (untuk aturan dependensi tier) ────
    // Melacak kombo (TowerType, baseTier) mana yang pernah
    // dimiliki pemain (dari draft atau beli di shop).
    // Dipakai oleh ShopManager::MeetsPrerequisite().
    //
    // Dikodekan sebagai: towerType * 10 + baseTier
    // contoh PLASMA(4) T2 → 42
    std::set<int> ownedTiers;

    // ── Siklus Hidup ─────────────────────────────────────────
    DeckManager();
    void InitPool();

    // ── Drafting (hanya T1) ───────────────────────────────
    void UpdateDrafting(const InputState& in);
    void DrawDrafting(AssetManager* assets = nullptr) const;
    bool IsDraftReady() const;
    void FinalizeDraft();
    bool IsTierAllowedInDraft(int tier) const;  // return true hanya untuk T1

    // ── Playing ──────────────────────────────────────────
    void UpdatePlaying(const InputState& in);
    void DrawPlaying(AssetManager* assets = nullptr) const;
    Card* GetSelectedCard();
    void DeselectAll();
    bool HasSelection() const;

    // ── Integrasi Shop ─────────────────────────────────
    bool CanAddCard() const;                    // hand.size() < maxHandSize
    void AddCardToHand(const CardDef& def);     // push ke tangan + daftarkan kepemilikan
    void UpgradeCapacity();                     // maxHandSize++, dibatasi di MAX_HAND_CAP
    bool CanUpgradeCapacity() const;            // maxHandSize < MAX_HAND_CAP
    int  SellCard(int slotIndex);               // hapus kartu, return harga jual (T1=$10, T2=$40, T3=$90)

    // ── Query Kepemilikan ────────────────────────────────
    void RegisterOwnership(TowerType type, int tier);
    bool OwnsType(TowerType type, int tier) const;

private:
    static int EncodeOwnership(TowerType type, int tier);
};
