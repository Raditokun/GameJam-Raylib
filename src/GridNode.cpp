#include "GridNode.h"
#include "Enemy.h"
#include "Projectile.h"

GridNode::GridNode() : row(0), col(0), worldPos({0,0}), isPathTile(false) {}

GridNode::GridNode(int r, int c, bool isPath)
    : row(r), col(c), isPathTile(isPath) {
    worldPos = {(float)(c*CELL_SIZE + CELL_SIZE/2), (float)(r*CELL_SIZE + CELL_SIZE/2)};
}

bool GridNode::CanPlaceTower(int tier) const {
    if (isPathTile) return false;
    if (towerStack.empty()) return true;
    return tier < GetTopTier();
}

void GridNode::PlaceTower(const Tower& tower) {
    towerStack.push_back(tower);
    CalculateSynergy();
}

int GridNode::SellTopTower() {
    if (towerStack.empty()) return 0;
    int refund = (int)(towerStack.back().baseStats.cost * 0.75f);
    towerStack.pop_back();
    CalculateSynergy();
    return refund;
}

void GridNode::UpdateAll(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& proj) {
    for (auto& t : towerStack) t.Update(dt, enemies, proj);
}

void GridNode::DrawAll() const {
    const float STACK_Y_OFFSET = 20.0f;
    for (int i = 0; i < (int)towerStack.size(); i++)
        towerStack[i].Draw(-i * STACK_Y_OFFSET);
}

bool GridNode::IsEmpty() const { return towerStack.empty(); }
int  GridNode::GetTopTier() const { return towerStack.empty() ? 0 : towerStack.back().baseTier; }

// ─── Upgrade Tower Teratas ──────────────────────────────────
bool GridNode::CanUpgradeTop() const {
    if (towerStack.empty()) return false;
    return towerStack.back().CanUpgrade();
}

int GridNode::GetTopUpgradeCost() const {
    if (towerStack.empty()) return 0;
    return towerStack.back().GetUpgradeCost();
}

bool GridNode::UpgradeTopTower(int& currency) {
    if (towerStack.empty()) return false;
    Tower& top = towerStack.back();
    if (!top.CanUpgrade()) return false;
    int cost = top.GetUpgradeCost();
    if (currency < cost) return false;
    currency -= cost;
    top.Upgrade();
    CalculateSynergy(); // Hitung ulang seluruh stack karena kontribusi T2/T3 berubah
    return true;
}

// ─── Perhitungan Sinergi ────────────────────────────────
// Ini fungsi balancing inti.
//
// FASE 1: Kumpulkan kontribusi dari tower T2 (aditif) dan T3 (multiplikatif).
// FASE 2: Hitung bonus kombo tipe (FREEZE, TESLA+PLASMA, tipe sama, evolusi penuh).
// FASE 3: Terapkan semuanya ke effectiveStats tiap tower via RecalcEffectiveStats().
//
// synergyMultiplier pada tiap tower mengandung:
//   (bonus kombo tipe) * (multiplier damage T3) + (bonus aditif T2 langsung dimasukkan ke effectiveStats)
//
void GridNode::CalculateSynergy() {
    if (towerStack.empty()) return;

    // ── Fase 1: Hitung kontribusi T2 dan T3 ─────────
    float stackDamageMultiplier = 1.0f;  // dari tower T3 (multiplikatif)
    float stackFireRateBonus    = 0;     // dari tower T2 (aditif)
    float stackRangeBonus       = 0;     // dari tower T2 (aditif)

    for (auto& t : towerStack) {
        if (t.baseTier == 3) {
            stackDamageMultiplier *= t.providedDamageMultiplier;
        }
        if (t.baseTier == 2) {
            stackFireRateBonus += t.providedFireRateBonus;
            stackRangeBonus   += t.providedRangeBonus;
        }
    }

    // ── Fase 2: Bonus kombo tipe ─────────────────────
    int typeCounts[5] = {0};
    bool hasFREEZE = false, hasTESLA = false, hasPLASMA = false;
    bool hasTier[4] = {false};
    TowerType firstType = towerStack[0].type;
    bool allSameType = true;

    for (auto& t : towerStack) {
        typeCounts[(int)t.type]++;
        if (t.type == TowerType::FREEZE) hasFREEZE = true;
        if (t.type == TowerType::TESLA)  hasTESLA  = true;
        if (t.type == TowerType::PLASMA) hasPLASMA = true;
        hasTier[t.baseTier] = true;
        if (t.type != firstType) allSameType = false;
    }
    bool fullEvolution = allSameType && hasTier[1] && hasTier[2] && hasTier[3];

    // ── Fase 3: Terapkan ke tiap tower ────────────────────
    for (auto& t : towerStack) {
        // Mulai dengan multiplier kombo tipe
        float comboMult = 1.0f;

        // Bonus tipe sama: +25% per tower tipe sama tambahan
        int sameCount = typeCounts[(int)t.type];
        if (sameCount > 1) comboMult += 0.25f * (sameCount - 1);

        // Kombo FREEZE: tower non-freeze dapat +20%
        if (hasFREEZE && t.type != TowerType::FREEZE) comboMult += 0.20f;

        // Kombo TESLA + PLASMA: keduanya dapat +30%
        if (hasTESLA && hasPLASMA && (t.type == TowerType::TESLA || t.type == TowerType::PLASMA))
            comboMult += 0.30f;

        // Evolusi penuh: 3 tier tipe sama → +50%
        if (fullEvolution) comboMult += 0.50f;

        // Multiplier sinergi akhir = kombo * amplifikasi T3
        t.synergyMultiplier = comboMult * stackDamageMultiplier;

        // Hitung ulang effective stats tower (skala-diri + multiplier sinergi)
        t.RecalcEffectiveStats();

        // Terapkan bonus aditif T2 DI ATAS skala-diri
        t.effectiveStats.fireRate += stackFireRateBonus;
        t.effectiveStats.range    += stackRangeBonus;
    }
}
