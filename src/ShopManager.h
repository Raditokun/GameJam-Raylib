#pragma once
#include <vector>
#include "Card.h"

// ─── ShopManager ────────────────────────────────────────
// Handles the between-wave shop screen. The shop appears
// every SHOP_WAVE_INTERVAL waves. It offers a rotating
// stock of T2/T3 tower cards plus a static "capacity
// upgrade" option.
//
// DEPENDENCY: ShopManager receives a const pointer to the
// player's owned cards (DeckManager::ownedTypes) to enforce
// the tier-dependency rule. It does NOT own the DeckManager.

constexpr int SHOP_WAVE_INTERVAL  = 3;     // shop appears every N waves
constexpr int SHOP_STOCK_SIZE     = 4;     // number of random tower cards offered
constexpr int CAPACITY_UPGRADE_COST = 150; // cost to add +1 hand slot

struct ShopItem {
    CardDef def;            // the tower card being sold
    int     price;          // currency cost
    bool    sold;           // true once purchased this visit
};

class DeckManager;          // forward declaration — no include needed

class ShopManager {
public:
    // ── Stock ─────────────────────────────────────────────
    std::vector<ShopItem> stock;       // random T2/T3 tower cards
    bool capacityUpgradeSold;          // true if already bought this visit

    // ── State ─────────────────────────────────────────────
    bool isOpen;                       // true while the shop screen is active

    // ── Lifecycle ─────────────────────────────────────────
    ShopManager();

    // Called when the shop opens: generate random stock
    void GenerateStock();

    // Called every frame while GameState::SHOP
    // Returns true when the player closes the shop (clicks "Continue")
    bool UpdateShop(int& currency, DeckManager& deck);

    void DrawShop(int currency, const DeckManager& deck) const;

    // ── Dependency Rule Check ────────────────────────────
    // Returns true if the player owns the required lower-tier
    // prerequisite for a given CardDef.
    // e.g. T2 Plasma requires owning T1 Plasma.
    //      T3 Plasma requires owning T2 Plasma.
    //      T1 always returns true (no prerequisite).
    static bool MeetsPrerequisite(const CardDef& item, const DeckManager& deck);

    // ── Helpers ───────────────────────────────────────────
    static bool ShouldOpenShop(int completedWaveIndex);  // checks if wave# triggers shop
};
