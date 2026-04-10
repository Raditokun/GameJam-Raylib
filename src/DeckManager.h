#pragma once
#include <vector>
#include <set>
#include "Card.h"

class AssetManager;

// ─── DeckManager (Overhauled) ───────────────────────────
// Now supports:
//  - Dynamic hand capacity (starts at 3, upgradeable to 5)
//  - T1-only drafting restriction
//  - Tier-dependency ownership tracking
//  - Adding cards from the ShopManager

class DeckManager {
public:
    // ── Draft Pool ────────────────────────────────────────
    std::vector<Card> pool;
    std::vector<int>  draftPicks;       // indices into pool
    bool draftComplete;

    // ── Active Hand (PLAYING) ────────────────────────────
    std::vector<Card> hand;
    int selectedHandIndex;

    // ── Capacity System ──────────────────────────────────
    int   maxHandSize;                  // starts at 3, upgradeable to MAX_HAND_CAP
    static constexpr int STARTING_HAND_CAP = 3;
    static constexpr int MAX_HAND_CAP      = 5;

    // ── Ownership Tracking (for tier dependency rule) ────
    // Tracks which (TowerType, baseTier) combos the player
    // has ever owned (drafted or purchased from shop).
    // Used by ShopManager::MeetsPrerequisite().
    //
    // Encoded as: towerType * 10 + baseTier
    // e.g. PLASMA(4) T2 → 42
    std::set<int> ownedTiers;

    // ── Lifecycle ─────────────────────────────────────────
    DeckManager();
    void InitPool();

    // ── Drafting (T1 only) ───────────────────────────────
    void UpdateDrafting();
    void DrawDrafting(AssetManager* assets = nullptr) const;
    bool IsDraftReady() const;
    void FinalizeDraft();
    bool IsTierAllowedInDraft(int tier) const;  // returns true only for T1

    // ── Playing ──────────────────────────────────────────
    void UpdatePlaying();
    void DrawPlaying(AssetManager* assets = nullptr) const;
    Card* GetSelectedCard();
    void DeselectAll();
    bool HasSelection() const;

    // ── Shop Integration ─────────────────────────────────
    bool CanAddCard() const;                    // hand.size() < maxHandSize
    void AddCardToHand(const CardDef& def);     // push to hand + register ownership
    void UpgradeCapacity();                     // maxHandSize++, capped at MAX_HAND_CAP
    bool CanUpgradeCapacity() const;            // maxHandSize < MAX_HAND_CAP
    int  SellCard(int slotIndex);               // remove card, return sell price (T1=$10, T2=$40, T3=$90)

    // ── Ownership Queries ────────────────────────────────
    void RegisterOwnership(TowerType type, int tier);
    bool OwnsType(TowerType type, int tier) const;

private:
    static int EncodeOwnership(TowerType type, int tier);
};
