#include "Card.h"
#include "AssetManager.h"
#include <cstdio>
#include <cmath>

Card::Card() : def({0, TowerType::LASER, 1}), selected(false), draftSelected(false) {}
Card::Card(CardDef d) : def(d), selected(false), draftSelected(false) {}

static void DrawTowerIcon(float cx, float cy, TowerType type, float scale, Color tc) {
    switch (type) {
    case TowerType::LASER:
        DrawTriangle({cx,cy-12*scale},{cx-10*scale,cy+6*scale},{cx+10*scale,cy+6*scale}, tc); break;
    case TowerType::MISSILE:
        DrawRectangle((int)(cx-8*scale),(int)(cy-5*scale),(int)(16*scale),(int)(12*scale), tc);
        DrawCircle((int)cx,(int)(cy-5*scale),(int)(6*scale), Fade(tc,0.8f)); break;
    case TowerType::FREEZE:
        DrawPoly({cx,cy}, 6, 12*scale, 30, tc); break;
    case TowerType::TESLA:
        DrawPoly({cx,cy}, 4, 12*scale, 45, tc); break;
    case TowerType::PLASMA:
        DrawPoly({cx,cy}, 5, 12*scale, 270, tc); break;
    }
}

void Card::DrawInHand(Rectangle r, AssetManager* assets) const {
    // ── Asset Integration Override ───────────────────────────
    if (def.towerType == TowerType::LASER && def.baseTier == 1 && assets) {
        Texture2D* tex = assets->Get("Tower_Laser__Card_T1");
        if (tex && tex->id > 0) {
            // Draw the entire card texture
            Rectangle src = { 0, 0, (float)tex->width, (float)tex->height };
            DrawTexturePro(*tex, src, r, {0,0}, 0.0f, WHITE);
            
            // Keep selection highlight if selected
            if (selected) {
                DrawRectangleLinesEx({r.x-2, r.y-2, r.width+4, r.height+4}, 3, COLOR_CARD_SEL);
            }
            return;
        }
    }

    Color tierCol = GetTierAccent(def.baseTier);
    Color border = selected ? COLOR_CARD_SEL : tierCol;
    float thick = selected ? 3.0f : 1.5f;

    DrawRectangleRec(r, COLOR_CARD_BG);
    DrawRectangle((int)r.x, (int)r.y, (int)r.width, 4, tierCol);
    DrawRectangleLinesEx(r, thick, border);
    if (selected)
        DrawRectangleLinesEx({r.x-2,r.y-2,r.width+4,r.height+4}, 1, Fade(COLOR_CARD_SEL,0.4f));

    Color tc = GetTowerColor(def.towerType);
    float iconScale = 1.0f + (def.baseTier-1)*0.25f;
    DrawTowerIcon(r.x+r.width/2, r.y+35, def.towerType, iconScale, tc);

    DrawText(GetCardName(def.towerType, def.baseTier), (int)(r.x+8), (int)(r.y+58), 11, COLOR_TEXT_MAIN);

    // Tier pips
    for (int i = 0; i < MAX_TIER; i++) {
        float sx = r.x+12+i*16, sy = r.y+78;
        if (i < def.baseTier) {
            DrawPoly({sx,sy}, 4, 6, 45, tierCol);
            DrawPoly({sx,sy}, 4, 3, 45, Fade(WHITE,0.5f));
        } else {
            DrawPolyLines({sx,sy}, 4, 6, 45, Fade(COLOR_TEXT_DIM,0.4f));
        }
    }

    TowerStats s = GetTierStats(def.towerType, def.baseTier);
    char buf[16]; snprintf(buf, sizeof(buf), "$%d", s.cost);
    DrawText(buf, (int)(r.x+10), (int)(r.y+100), 14, COLOR_CURRENCY);
}

void Card::DrawInDraft(Rectangle r, AssetManager* assets) const {
    // ── Asset Integration Override ───────────────────────────
    if (def.towerType == TowerType::LASER && def.baseTier == 1 && assets) {
        Texture2D* tex = assets->Get("Tower_Laser__Card_T1");
        if (tex && tex->id > 0) {
            // Draw the entire card texture
            Rectangle src = { 0, 0, (float)tex->width, (float)tex->height };
            DrawTexturePro(*tex, src, r, {0,0}, 0.0f, WHITE);
            
            // Keep selection highlight/text if picked
            if (draftSelected) {
                DrawRectangleLinesEx({r.x-3, r.y-3, r.width+6, r.height+6}, 2, Fade(COLOR_CARD_SEL,0.5f));
                DrawText("PICKED", (int)(r.x+r.width-55), (int)(r.y+8), 10, COLOR_CARD_SEL);
            }
            return;
        }
    }

    Color tierCol = GetTierAccent(def.baseTier);
    Color border = draftSelected ? COLOR_CARD_SEL : tierCol;
    float thick = draftSelected ? 3.0f : 1.5f;

    DrawRectangleRec(r, draftSelected ? CLITERAL(Color){35,30,55,255} : COLOR_CARD_BG);
    DrawRectangle((int)r.x, (int)r.y, (int)r.width, 5, tierCol);
    DrawRectangleLinesEx(r, thick, border);

    if (draftSelected) {
        DrawRectangleLinesEx({r.x-3,r.y-3,r.width+6,r.height+6}, 2, Fade(COLOR_CARD_SEL,0.5f));
        DrawText("PICKED", (int)(r.x+r.width-55), (int)(r.y+8), 10, COLOR_CARD_SEL);
    }

    Color tc = GetTowerColor(def.towerType);
    float iconScale = 1.0f + (def.baseTier-1)*0.3f;
    DrawTowerIcon(r.x+r.width/2, r.y+45, def.towerType, iconScale, tc);

    DrawText(GetCardName(def.towerType, def.baseTier), (int)(r.x+10), (int)(r.y+72), 12, COLOR_TEXT_MAIN);

    // Tier label
    char tierBuf[8]; snprintf(tierBuf, sizeof(tierBuf), "T%d", def.baseTier);
    DrawText(tierBuf, (int)(r.x+10), (int)(r.y+90), 14, tierCol);

    // Cost
    TowerStats s = GetTierStats(def.towerType, def.baseTier);
    char costBuf[16]; snprintf(costBuf, sizeof(costBuf), "$%d", s.cost);
    DrawText(costBuf, (int)(r.x+10), (int)(r.y+110), 14, COLOR_CURRENCY);

    // Stats preview
    char dmgBuf[16]; snprintf(dmgBuf, sizeof(dmgBuf), "DMG:%.0f", s.damage);
    DrawText(dmgBuf, (int)(r.x+10), (int)(r.y+130), 10, COLOR_TEXT_DIM);
    char rngBuf[16]; snprintf(rngBuf, sizeof(rngBuf), "RNG:%.0f", s.range);
    DrawText(rngBuf, (int)(r.x+70), (int)(r.y+130), 10, COLOR_TEXT_DIM);
    char spdBuf[16]; snprintf(spdBuf, sizeof(spdBuf), "SPD:%.1f", s.fireRate);
    DrawText(spdBuf, (int)(r.x+10), (int)(r.y+145), 10, COLOR_TEXT_DIM);
}

int Card::GetPlacementCost() const { return GetTierStats(def.towerType, def.baseTier).cost; }
