#include "DeckManager.h"
#include <algorithm>
#include <cstdio>

DeckManager::DeckManager()
    : draftComplete(false), selectedHandIndex(-1), maxHandSize(STARTING_HAND_CAP) {}

void DeckManager::InitPool() {
    pool.clear();
    draftPicks.clear();
    hand.clear();
    ownedTiers.clear();
    draftComplete = false;
    selectedHandIndex = -1;
    maxHandSize = STARTING_HAND_CAP;
    for (int i = 0; i < POOL_SIZE; i++)
        pool.emplace_back(CARD_POOL[i]);
}

// ─── Ownership ──────────────────────────────────────────

int DeckManager::EncodeOwnership(TowerType type, int tier) {
    return (int)type * 10 + tier;
}

void DeckManager::RegisterOwnership(TowerType type, int tier) {
    ownedTiers.insert(EncodeOwnership(type, tier));
}

bool DeckManager::OwnsType(TowerType type, int tier) const {
    return ownedTiers.count(EncodeOwnership(type, tier)) > 0;
}

// ─── Drafting (T1 only) ─────────────────────────────────

bool DeckManager::IsTierAllowedInDraft(int tier) const {
    return tier == 1;
}

void DeckManager::UpdateDrafting() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();
        for (int i = 0; i < (int)pool.size(); i++) {
            Rectangle r = GetDraftSlotRect(i);
            if (CheckCollisionPointRec(mp, r)) {
                // Only allow T1 cards in draft
                if (!IsTierAllowedInDraft(pool[i].def.baseTier)) break;

                if (pool[i].draftSelected) {
                    pool[i].draftSelected = false;
                    draftPicks.erase(std::remove(draftPicks.begin(), draftPicks.end(), i), draftPicks.end());
                } else if ((int)draftPicks.size() < maxHandSize) {
                    pool[i].draftSelected = true;
                    draftPicks.push_back(i);
                }
                break;
            }
        }
    }
}

void DeckManager::DrawDrafting(AssetManager* assets) const {
    const char* title = "DRAFT YOUR DECK";
    int tw = MeasureText(title, 40);
    DrawText(title, (SCREEN_WIDTH-tw)/2, 30, 40, COLOR_TEXT_MAIN);

    char sub[64];
    snprintf(sub, sizeof(sub), "Select %d T1 cards  (%d / %d)", maxHandSize, (int)draftPicks.size(), maxHandSize);
    int sw = MeasureText(sub, 18);
    DrawText(sub, (SCREEN_WIDTH-sw)/2, 80, 18,
             (int)draftPicks.size() == maxHandSize ? COLOR_CARD_SEL : COLOR_TEXT_DIM);

    // Column headers
    for (int col = 0; col < DRAFT_COLS; col++) {
        Rectangle first = GetDraftSlotRect(col * 3);
        const char* name = GetTowerName(CARD_POOL[col*3].towerType);
        Color tc = GetTowerColor(CARD_POOL[col*3].towerType);
        int nw = MeasureText(name, 14);
        DrawText(name, (int)(first.x + first.width/2 - nw/2), (int)(first.y - 20), 14, tc);
    }

    // Draw pool cards — grey out non-T1
    for (int i = 0; i < (int)pool.size(); i++) {
        Rectangle r = GetDraftSlotRect(i);
        if (!IsTierAllowedInDraft(pool[i].def.baseTier)) {
            // Locked card
            DrawRectangleRec(r, CLITERAL(Color){20, 15, 35, 255});
            DrawRectangleLinesEx(r, 1, Fade(COLOR_TEXT_DIM, 0.3f));
            DrawText("LOCKED", (int)(r.x + r.width/2 - 28), (int)(r.y + r.height/2 - 6), 12, Fade(COLOR_TEXT_DIM, 0.5f));
        } else {
            pool[i].DrawInDraft(r, assets);
        }
    }

    // START button
    if (IsDraftReady()) {
        float btnW = 220, btnH = 50;
        float btnX = (SCREEN_WIDTH - btnW) / 2;
        float btnY = SCREEN_HEIGHT - 100;
        float pulse = 0.7f + 0.3f * sinf((float)GetTime() * 3.0f);

        DrawRectangle((int)btnX, (int)btnY, (int)btnW, (int)btnH, Fade(COLOR_CARD_SEL, 0.2f));
        DrawRectangleLinesEx({btnX, btnY, btnW, btnH}, 2, Fade(COLOR_CARD_SEL, pulse));
        const char* btnTxt = "START GAME";
        int btw = MeasureText(btnTxt, 24);
        DrawText(btnTxt, (int)(btnX + btnW/2 - btw/2), (int)(btnY + 13), 24, Fade(COLOR_CARD_SEL, pulse));
    }
}

bool DeckManager::IsDraftReady() const { return (int)draftPicks.size() == maxHandSize; }

void DeckManager::FinalizeDraft() {
    hand.clear();
    for (int idx : draftPicks) {
        Card c = pool[idx];
        c.draftSelected = false;
        c.selected = false;
        hand.push_back(c);
        RegisterOwnership(c.def.towerType, c.def.baseTier);
    }
    draftComplete = true;
    selectedHandIndex = -1;
}

// ─── Playing ────────────────────────────────────────────

void DeckManager::UpdatePlaying() {
    for (int i = 0; i < (int)hand.size(); i++) {
        if (IsKeyPressed(KEY_ONE + i)) {
            DeselectAll();
            hand[i].selected = true;
            selectedHandIndex = i;
            return;
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();
        for (int i = 0; i < (int)hand.size(); i++) {
            Rectangle r = GetHandSlotRect(i);
            if (CheckCollisionPointRec(mp, r)) {
                DeselectAll();
                hand[i].selected = true;
                selectedHandIndex = i;
                return;
            }
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) DeselectAll();
}

void DeckManager::DrawPlaying(AssetManager* assets) const {
    for (int i = 0; i < (int)hand.size(); i++)
        hand[i].DrawInHand(GetHandSlotRect(i), assets);
}

Card* DeckManager::GetSelectedCard() {
    if (selectedHandIndex < 0 || selectedHandIndex >= (int)hand.size()) return nullptr;
    return &hand[selectedHandIndex];
}

void DeckManager::DeselectAll() {
    for (auto& c : hand) c.selected = false;
    selectedHandIndex = -1;
}

bool DeckManager::HasSelection() const { return selectedHandIndex >= 0; }

// ─── Shop Integration ───────────────────────────────────

bool DeckManager::CanAddCard() const {
    return (int)hand.size() < maxHandSize;
}

void DeckManager::AddCardToHand(const CardDef& def) {
    if (!CanAddCard()) return;
    Card c(def);
    c.selected = false;
    c.draftSelected = false;
    hand.push_back(c);
    RegisterOwnership(def.towerType, def.baseTier);
}

void DeckManager::UpgradeCapacity() {
    if (maxHandSize < MAX_HAND_CAP)
        maxHandSize++;
}

bool DeckManager::CanUpgradeCapacity() const {
    return maxHandSize < MAX_HAND_CAP;
}
