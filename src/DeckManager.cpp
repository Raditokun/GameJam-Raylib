#include "DeckManager.h"
#include <algorithm>
#include <cstdio>

DeckManager::DeckManager() : draftComplete(false), selectedHandIndex(-1) {}

void DeckManager::InitPool() {
    pool.clear();
    draftPicks.clear();
    hand.clear();
    draftComplete = false;
    selectedHandIndex = -1;
    for (int i = 0; i < POOL_SIZE; i++)
        pool.emplace_back(CARD_POOL[i]);
}

// ─── Drafting ───────────────────────────────────────────

void DeckManager::UpdateDrafting() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();
        for (int i = 0; i < (int)pool.size(); i++) {
            Rectangle r = GetDraftSlotRect(i);
            if (CheckCollisionPointRec(mp, r)) {
                if (pool[i].draftSelected) {
                    // Deselect
                    pool[i].draftSelected = false;
                    draftPicks.erase(std::remove(draftPicks.begin(), draftPicks.end(), i), draftPicks.end());
                } else if ((int)draftPicks.size() < HAND_SIZE) {
                    // Select
                    pool[i].draftSelected = true;
                    draftPicks.push_back(i);
                }
                break;
            }
        }
    }
}

void DeckManager::DrawDrafting() const {
    // Title
    const char* title = "DRAFT YOUR DECK";
    int tw = MeasureText(title, 40);
    DrawText(title, (SCREEN_WIDTH-tw)/2, 30, 40, COLOR_TEXT_MAIN);

    // Subtitle
    char sub[64];
    snprintf(sub, sizeof(sub), "Select %d cards  (%d / %d)", HAND_SIZE, (int)draftPicks.size(), HAND_SIZE);
    int sw = MeasureText(sub, 18);
    DrawText(sub, (SCREEN_WIDTH-sw)/2, 80, 18,
             (int)draftPicks.size() == HAND_SIZE ? COLOR_CARD_SEL : COLOR_TEXT_DIM);

    // Column headers (tower type names)
    for (int col = 0; col < DRAFT_COLS; col++) {
        Rectangle first = GetDraftSlotRect(col * 3);
        const char* name = GetTowerName(CARD_POOL[col*3].towerType);
        Color tc = GetTowerColor(CARD_POOL[col*3].towerType);
        int nw = MeasureText(name, 14);
        DrawText(name, (int)(first.x + first.width/2 - nw/2), (int)(first.y - 20), 14, tc);
    }

    // Draw all pool cards
    for (int i = 0; i < (int)pool.size(); i++) {
        Rectangle r = GetDraftSlotRect(i);
        pool[i].DrawInDraft(r);
    }

    // START button (only when exactly 5 picked)
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

bool DeckManager::IsDraftReady() const { return (int)draftPicks.size() == HAND_SIZE; }

void DeckManager::FinalizeDraft() {
    hand.clear();
    for (int idx : draftPicks) {
        Card c = pool[idx];
        c.draftSelected = false;
        c.selected = false;
        hand.push_back(c);
    }
    draftComplete = true;
    selectedHandIndex = -1;
}

// ─── Playing ────────────────────────────────────────────

void DeckManager::UpdatePlaying() {
    // Number keys 1-5
    for (int i = 0; i < (int)hand.size(); i++) {
        if (IsKeyPressed(KEY_ONE + i)) {
            DeselectAll();
            hand[i].selected = true;
            selectedHandIndex = i;
            return;
        }
    }

    // Click on hand card to select
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

void DeckManager::DrawPlaying() const {
    for (int i = 0; i < (int)hand.size(); i++)
        hand[i].DrawInHand(GetHandSlotRect(i));
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
