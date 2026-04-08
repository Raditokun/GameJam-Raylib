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

// ─── Upgrade Top Tower ──────────────────────────────────
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
    CalculateSynergy(); // Recalc entire stack since T2/T3 contributions changed
    return true;
}

// ─── Synergy Calculation ────────────────────────────────
// This is the core balancing function.
//
// PHASE 1: Gather contributions from T2 (additive) and T3 (multiplicative) towers.
// PHASE 2: Compute base type-combo bonuses (FREEZE, TESLA+PLASMA, same-type, full evolution).
// PHASE 3: Apply everything to each tower's effectiveStats via RecalcEffectiveStats().
//
// The synergyMultiplier on each tower encodes:
//   (type-combo bonus) * (T3 damage multipliers) + (T2 additive bonuses are baked into effectiveStats directly)
//
void GridNode::CalculateSynergy() {
    if (towerStack.empty()) return;

    // ── Phase 1: Tally T2 and T3 contributions ─────────
    float stackDamageMultiplier = 1.0f;  // from T3 towers (multiplicative)
    float stackFireRateBonus    = 0;     // from T2 towers (additive)
    float stackRangeBonus       = 0;     // from T2 towers (additive)

    for (auto& t : towerStack) {
        if (t.baseTier == 3) {
            stackDamageMultiplier *= t.providedDamageMultiplier;
        }
        if (t.baseTier == 2) {
            stackFireRateBonus += t.providedFireRateBonus;
            stackRangeBonus   += t.providedRangeBonus;
        }
    }

    // ── Phase 2: Type-combo bonuses ─────────────────────
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

    // ── Phase 3: Apply to each tower ────────────────────
    for (auto& t : towerStack) {
        // Start with type-combo multiplier
        float comboMult = 1.0f;

        // Same-type bonus: +25% per additional same-type tower
        int sameCount = typeCounts[(int)t.type];
        if (sameCount > 1) comboMult += 0.25f * (sameCount - 1);

        // FREEZE combo: non-freeze towers get +20%
        if (hasFREEZE && t.type != TowerType::FREEZE) comboMult += 0.20f;

        // TESLA + PLASMA combo: both get +30%
        if (hasTESLA && hasPLASMA && (t.type == TowerType::TESLA || t.type == TowerType::PLASMA))
            comboMult += 0.30f;

        // Full evolution: all 3 tiers of same type → +50%
        if (fullEvolution) comboMult += 0.50f;

        // Final synergy multiplier = combo * T3 amplification
        t.synergyMultiplier = comboMult * stackDamageMultiplier;

        // Recalc the tower's own effective stats (self-scaling + synergy multiplier)
        t.RecalcEffectiveStats();

        // Apply T2 additive bonuses ON TOP of self-scaling
        t.effectiveStats.fireRate += stackFireRateBonus;
        t.effectiveStats.range    += stackRangeBonus;
    }
}
