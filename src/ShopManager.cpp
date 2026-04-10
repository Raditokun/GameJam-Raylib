#include "ShopManager.h"
#include "DeckManager.h"
#include <cstdlib>
#include <cstdio>
#include <cmath>

ShopManager::ShopManager() : capacityUpgradeSold(false), isOpen(false) {}

// ─── Stock Generation ───────────────────────────────────

void ShopManager::GenerateStock() {
    stock.clear();
    capacityUpgradeSold = false;
    isOpen = true;

    // Collect all T2/T3 card defs from the global pool
    std::vector<int> candidates;
    for (int i = 0; i < POOL_SIZE; i++) {
        if (CARD_POOL[i].baseTier >= 2)
            candidates.push_back(i);
    }

    // Shuffle and pick up to SHOP_STOCK_SIZE
    for (int i = (int)candidates.size() - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = tmp;
    }

    int count = (SHOP_STOCK_SIZE < (int)candidates.size()) ? SHOP_STOCK_SIZE : (int)candidates.size();
    for (int i = 0; i < count; i++) {
        CardDef def = CARD_POOL[candidates[i]];
        TowerStats s = GetTierStats(def.towerType, def.baseTier);
        stock.push_back({def, s.cost, false});
    }
}

// ─── Prerequisite Check ─────────────────────────────────

bool ShopManager::MeetsPrerequisite(const CardDef& item, const DeckManager& deck) {
    if (item.baseTier <= 1) return true;
    // Must own the tier directly below
    return deck.OwnsType(item.towerType, item.baseTier - 1);
}

// ─── Shop Trigger ───────────────────────────────────────

bool ShopManager::ShouldOpenShop(int completedWaveIndex) {
    // completedWaveIndex is 0-based; wave 3 = index 2, wave 6 = index 5, etc.
    return (completedWaveIndex + 1) % SHOP_WAVE_INTERVAL == 0;
}

// ─── Update (returns true when shop closes) ─────────────

bool ShopManager::UpdateShop(int& currency, DeckManager& deck) {
    if (!isOpen) return true;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();

        // ── Tower item buttons ───────────────────────────
        float cardW = 200, cardH = 160, spacing = 20;
        float totalW = stock.size() * cardW + (stock.size()-1) * spacing;
        float startX = (SCREEN_WIDTH - totalW) / 2.0f;
        float startY = 260.0f;

        for (int i = 0; i < (int)stock.size(); i++) {
            Rectangle r = {startX + i*(cardW+spacing), startY, cardW, cardH};
            if (CheckCollisionPointRec(mp, r) && !stock[i].sold) {
                bool meetsReq = MeetsPrerequisite(stock[i].def, deck);
                bool canAfford = currency >= stock[i].price;
                bool hasRoom = deck.CanAddCard();
                if (meetsReq && canAfford && hasRoom) {
                    currency -= stock[i].price;
                    deck.AddCardToHand(stock[i].def);
                    stock[i].sold = true;
                }
            }
        }

        // ── Capacity upgrade button ──────────────────────
        float capBtnW = 280, capBtnH = 50;
        float capBtnX = (SCREEN_WIDTH - capBtnW) / 2.0f;
        float capBtnY = startY + cardH + 40;
        Rectangle capRect = {capBtnX, capBtnY, capBtnW, capBtnH};

        if (CheckCollisionPointRec(mp, capRect) && !capacityUpgradeSold && deck.CanUpgradeCapacity()) {
            if (currency >= CAPACITY_UPGRADE_COST) {
                currency -= CAPACITY_UPGRADE_COST;
                deck.UpgradeCapacity();
                capacityUpgradeSold = true;
            }
        }

        // ── Continue button ──────────────────────────────
        float btnW = 240, btnH = 55;
        float btnX = (SCREEN_WIDTH - btnW) / 2.0f;
        float btnY = SCREEN_HEIGHT - 240;
        Rectangle btnRect = {btnX, btnY, btnW, btnH};

        if (CheckCollisionPointRec(mp, btnRect)) {
            isOpen = false;
            return true;
        }
    }

    // ── Right-click: Sell card from hand ─────────────────
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        Vector2 mp = GetMousePosition();
        for (int i = 0; i < (int)deck.hand.size(); i++) {
            Rectangle r = GetHandSlotRect(i);
            // Offset cards to shop hand area at bottom
            r.y = SCREEN_HEIGHT - 160;
            if (CheckCollisionPointRec(mp, r)) {
                currency += deck.SellCard(i);
                break;
            }
        }
    }

    return false;
}

// ─── Draw ───────────────────────────────────────────────

void ShopManager::DrawShop(int currency, const DeckManager& deck) const {
    // Background overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.85f));

    // Title
    const char* title = "TOWER SHOP";
    int tw = MeasureText(title, 48);
    DrawText(title, (SCREEN_WIDTH-tw)/2, 40, 48, COLOR_CURRENCY);

    // Subtitle
    char sub[64];
    snprintf(sub, sizeof(sub), "Currency: $%d    Hand: %d/%d", currency, (int)deck.hand.size(), deck.maxHandSize);
    int sw = MeasureText(sub, 18);
    DrawText(sub, (SCREEN_WIDTH-sw)/2, 100, 18, COLOR_TEXT_MAIN);

    // ── Tower cards ──────────────────────────────────────
    float cardW = 200, cardH = 160, spacing = 20;
    float totalW = stock.size() * cardW + (stock.size()-1) * spacing;
    float startX = (SCREEN_WIDTH - totalW) / 2.0f;
    float startY = 260.0f;

    // Section label
    DrawText("Available Towers (T2/T3)", (int)startX, (int)(startY - 30), 16, COLOR_TEXT_DIM);

    for (int i = 0; i < (int)stock.size(); i++) {
        Rectangle r = {startX + i*(cardW+spacing), startY, cardW, cardH};
        const ShopItem& item = stock[i];
        bool meetsReq = MeetsPrerequisite(item.def, deck);
        bool canAfford = currency >= item.price;
        bool hasRoom = deck.CanAddCard();
        bool canBuy = meetsReq && canAfford && hasRoom && !item.sold;

        Color tierCol = GetTierAccent(item.def.baseTier);
        Color towerCol = GetTowerColor(item.def.towerType);

        if (item.sold) {
            // Sold overlay
            DrawRectangleRec(r, CLITERAL(Color){15, 12, 25, 240});
            DrawRectangleLinesEx(r, 1, Fade(COLOR_TEXT_DIM, 0.3f));
            const char* soldTxt = "SOLD";
            int stw = MeasureText(soldTxt, 24);
            DrawText(soldTxt, (int)(r.x + r.width/2 - stw/2), (int)(r.y + r.height/2 - 12), 24,
                     Fade(COLOR_CARD_SEL, 0.6f));
        } else {
            // Card background
            DrawRectangleRec(r, canBuy ? COLOR_CARD_BG : CLITERAL(Color){20, 15, 35, 255});
            DrawRectangle((int)r.x, (int)r.y, (int)r.width, 5, tierCol);
            DrawRectangleLinesEx(r, canBuy ? 2.0f : 1.0f, canBuy ? tierCol : Fade(COLOR_TEXT_DIM, 0.4f));

            // Tower name
            DrawText(GetCardName(item.def.towerType, item.def.baseTier),
                     (int)(r.x+12), (int)(r.y+16), 14, towerCol);

            // Tier label
            char tierBuf[8];
            snprintf(tierBuf, sizeof(tierBuf), "T%d", item.def.baseTier);
            DrawText(tierBuf, (int)(r.x+12), (int)(r.y+36), 12, tierCol);

            // Stats
            TowerStats s = GetTierStats(item.def.towerType, item.def.baseTier);
            char buf[32];
            snprintf(buf, sizeof(buf), "DMG:%.0f  RNG:%.0f", s.damage, s.range);
            DrawText(buf, (int)(r.x+12), (int)(r.y+56), 10, COLOR_TEXT_DIM);
            snprintf(buf, sizeof(buf), "SPD:%.1f", s.fireRate);
            DrawText(buf, (int)(r.x+12), (int)(r.y+70), 10, COLOR_TEXT_DIM);

            // Price
            snprintf(buf, sizeof(buf), "$%d", item.price);
            DrawText(buf, (int)(r.x+12), (int)(r.y+92), 18,
                     canAfford ? COLOR_CURRENCY : Fade(COLOR_HEALTH_BAR, 0.8f));

            // Status messages
            if (!meetsReq) {
                char req[48];
                snprintf(req, sizeof(req), "REQUIRES: %s T%d",
                         GetTowerName(item.def.towerType), item.def.baseTier - 1);
                DrawText(req, (int)(r.x+12), (int)(r.y+120), 10, COLOR_HEALTH_BAR);
                DrawText("LOCKED", (int)(r.x+12), (int)(r.y+136), 12,
                         Fade(COLOR_HEALTH_BAR, 0.7f));
            } else if (!hasRoom) {
                DrawText("HAND FULL", (int)(r.x+12), (int)(r.y+120), 12,
                         Fade(COLOR_HEALTH_BAR, 0.7f));
            } else if (!canAfford) {
                DrawText("NOT ENOUGH $", (int)(r.x+12), (int)(r.y+120), 12,
                         Fade(COLOR_HEALTH_BAR, 0.6f));
            } else {
                DrawText("CLICK TO BUY", (int)(r.x+12), (int)(r.y+120), 12,
                         Fade(COLOR_CARD_SEL, 0.8f));
            }
        }
    }

    // ── Capacity Upgrade ─────────────────────────────────
    float capBtnW = 280, capBtnH = 50;
    float capBtnX = (SCREEN_WIDTH - capBtnW) / 2.0f;
    float capBtnY = startY + cardH + 40;

    if (capacityUpgradeSold) {
        DrawRectangle((int)capBtnX, (int)capBtnY, (int)capBtnW, (int)capBtnH,
                      CLITERAL(Color){15,12,25,240});
        DrawRectangleLinesEx({capBtnX, capBtnY, capBtnW, capBtnH}, 1, Fade(COLOR_TEXT_DIM, 0.3f));
        const char* txt = "CAPACITY UPGRADED!";
        int tw2 = MeasureText(txt, 16);
        DrawText(txt, (int)(capBtnX + capBtnW/2 - tw2/2), (int)(capBtnY + 17), 16,
                 Fade(COLOR_CARD_SEL, 0.6f));
    } else if (!deck.CanUpgradeCapacity()) {
        DrawRectangle((int)capBtnX, (int)capBtnY, (int)capBtnW, (int)capBtnH,
                      CLITERAL(Color){15,12,25,240});
        DrawRectangleLinesEx({capBtnX, capBtnY, capBtnW, capBtnH}, 1, Fade(COLOR_TEXT_DIM, 0.3f));
        const char* txt = "MAX CAPACITY REACHED";
        int tw2 = MeasureText(txt, 14);
        DrawText(txt, (int)(capBtnX + capBtnW/2 - tw2/2), (int)(capBtnY + 18), 14, COLOR_TEXT_DIM);
    } else {
        bool canAffordCap = currency >= CAPACITY_UPGRADE_COST;
        Color btnCol = canAffordCap ? COLOR_CARD_SEL : COLOR_TEXT_DIM;
        DrawRectangle((int)capBtnX, (int)capBtnY, (int)capBtnW, (int)capBtnH,
                      Fade(btnCol, 0.15f));
        DrawRectangleLinesEx({capBtnX, capBtnY, capBtnW, capBtnH}, 2, Fade(btnCol, 0.8f));

        char capTxt[64];
        snprintf(capTxt, sizeof(capTxt), "+1 Hand Slot (%d -> %d)  $%d",
                 deck.maxHandSize, deck.maxHandSize + 1, CAPACITY_UPGRADE_COST);
        int ctw = MeasureText(capTxt, 14);
        DrawText(capTxt, (int)(capBtnX + capBtnW/2 - ctw/2), (int)(capBtnY + 18), 14,
                 canAffordCap ? COLOR_CURRENCY : Fade(COLOR_HEALTH_BAR, 0.6f));
    }

    // ── Continue Button ──────────────────────────────────
    float btnW = 240, btnH = 55;
    float btnX = (SCREEN_WIDTH - btnW) / 2.0f;
    float btnY = SCREEN_HEIGHT - 240;
    float pulse = 0.7f + 0.3f * sinf((float)GetTime() * 3.0f);

    DrawRectangle((int)btnX, (int)btnY, (int)btnW, (int)btnH, Fade(COLOR_CARD_SEL, 0.2f));
    DrawRectangleLinesEx({btnX, btnY, btnW, btnH}, 2, Fade(COLOR_CARD_SEL, pulse));
    const char* btnTxt = "RESUME GAME";
    int btw = MeasureText(btnTxt, 26);
    DrawText(btnTxt, (int)(btnX + btnW/2 - btw/2), (int)(btnY + 14), 26,
             Fade(COLOR_CARD_SEL, pulse));

    // ── Your Hand (Sell Cards) ───────────────────────────
    if (!deck.hand.empty()) {
        DrawText("YOUR HAND  [Right-Click to Sell]", 20, SCREEN_HEIGHT - 190, 14, COLOR_TEXT_DIM);
        for (int i = 0; i < (int)deck.hand.size(); i++) {
            Rectangle r = GetHandSlotRect(i);
            r.y = SCREEN_HEIGHT - 160;
            deck.hand[i].DrawInHand(r);

            // Sell price label
            int tier = deck.hand[i].def.baseTier;
            int sellPrice = (tier == 1) ? 10 : (tier == 2) ? 40 : 90;
            char sBuf[16];
            snprintf(sBuf, sizeof(sBuf), "SELL $%d", sellPrice);
            int stw = MeasureText(sBuf, 9);
            DrawText(sBuf, (int)(r.x + r.width/2 - stw/2), (int)(r.y + r.height + 4), 9,
                     Fade(COLOR_HEALTH_BAR, 0.7f));
        }
    }
}
