#include "Game.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdlib>

Game::Game()
    : state(GameState::DRAFTING), currency(STARTING_CURRENCY),
      playerHealth(hero.currentHP), camera({0}), screenShakeTimer(0)
{
    memset(pathCells, false, sizeof(pathCells));
}

void Game::Init() {
    if (!IsWindowReady()) {
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ALIEN TD - Deck Builder Tower Defense");
        SetTargetFPS(TARGET_FPS);
    }

    // Build path
    pathPoints.clear();
    for (int i = 0; i < PATH_WP_COUNT; i++)
        pathPoints.push_back({(float)(PATH_WP[i][0]*CELL_SIZE+CELL_SIZE/2),(float)(PATH_WP[i][1]*CELL_SIZE+CELL_SIZE/2)});

    memset(pathCells, false, sizeof(pathCells));
    for (int i = 0; i < PATH_WP_COUNT-1; i++) {
        int c1=PATH_WP[i][0],r1=PATH_WP[i][1],c2=PATH_WP[i+1][0],r2=PATH_WP[i+1][1];
        if (c1==c2) { int lo=std::min(r1,r2),hi=std::max(r1,r2); for(int r=lo;r<=hi;r++) pathCells[r][c1]=true; }
        else        { int lo=std::min(c1,c2),hi=std::max(c1,c2); for(int c=lo;c<=hi;c++) pathCells[r1][c]=true; }
    }

    InitGrid();
    deck.InitPool();
    waves.Init();
    hero.Init(pathPoints.back());

    // Camera init
    camera.offset = {0, 0};
    camera.target = {0, 0};
    camera.rotation = 0;
    camera.zoom = 1.0f;
    screenShakeTimer = 0;

    // ── Load Assets (must be after window init) ──────────
    assets.Load("proj_laser", "assets/laser_partikel.png");
    assets.Load("proj_missile", "assets/missiles_partikel.png");
    assets.Load("proj_freeze", "assets/frezze_partikel.png");
    assets.Load("proj_tesla", "assets/Tesla_Partikel.png");
    assets.Load("proj_plasma", "assets/Plasma_Partikel.png");
    assets.Load("Tower_Laser__Card_T1", "assets/LaserT1Card.png");

    state = GameState::DRAFTING;
    currency = STARTING_CURRENCY;
    enemies.clear();
    projectiles.clear();
}

void Game::Shutdown() {
    assets.UnloadAll();
}

void Game::InitGrid() {
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            grid[r][c] = GridNode(r, c, pathCells[r][c]);
}

// ─── Update ─────────────────────────────────────────────

void Game::Update(float dt) {
    if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

    if (state == GameState::GAME_OVER || state == GameState::VICTORY) {
        if (IsKeyPressed(KEY_R)) Init();
        return;
    }
    
    hero.Update(dt, state == GameState::PLAYING && waves.waveActive);

    if (state == GameState::DRAFTING) { UpdateDrafting(); return; }
    if (state == GameState::SHOP)    { UpdateShop(); return; }

    // PLAYING state
    deck.UpdatePlaying();
    HandleInput();
    waves.Update(dt, enemies, pathPoints);

    for (int r=0;r<GRID_ROWS;r++)
        for (int c=0;c<GRID_COLS;c++)
            grid[r][c].UpdateAll(dt, enemies, projectiles);

    for (auto& e : enemies) e.Update(dt, pathPoints);
    UpdateProjectiles(dt);
    CheckEnemyReachedBase();
    CleanupDead();

    // ── Hero Ultimate: one-frame massive damage ──────────
    if (hero.isUltFiring) {
        for (auto& e : enemies) {
            if (!e.alive) continue;
            e.hp -= hero.ultDamage;
            if (e.hp <= 0) { e.alive = false; currency += e.reward; }
        }
        hero.isUltFiring = false; // damage only on first frame
    }

    // ── Screen shake timer ───────────────────────────────
    if (screenShakeTimer > 0) {
        screenShakeTimer -= dt;
        float intensity = screenShakeTimer * 20.0f; // decays from 10 to 0
        if (intensity > 10.0f) intensity = 10.0f;
        camera.offset.x = (float)((rand() % 200 - 100) / 100.0f) * intensity;
        camera.offset.y = (float)((rand() % 200 - 100) / 100.0f) * intensity;
    } else {
        camera.offset.x = 0;
        camera.offset.y = 0;
    }

    // ── Wave-end shop check ──────────────────────────────
    CheckWaveEndShop();

    if (waves.AllWavesComplete() && waves.IsWaveCleared(enemies))
        state = GameState::VICTORY;
}

void Game::UpdateDrafting() {
    deck.UpdateDrafting();

    if (deck.IsDraftReady() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        float btnW=220, btnH=50, btnX=(SCREEN_WIDTH-btnW)/2, btnY=SCREEN_HEIGHT-100;
        Vector2 mp = GetMousePosition();
        if (CheckCollisionPointRec(mp, {btnX,btnY,btnW,btnH})) {
            deck.FinalizeDraft();
            state = GameState::PLAYING;
        }
    }
}

void Game::CheckWaveEndShop() {
    if (!waves.waveActive && waves.currentWave >= 0) {
        if (ShopManager::ShouldOpenShop(waves.currentWave) && !shop.isOpen) {
            bool cleared = true;
            for (auto& e : enemies) if (e.alive) { cleared = false; break; }
            if (cleared) {
                shop.GenerateStock();
                state = GameState::SHOP;
            }
        }
    }
}

void Game::UpdateShop() {
    if (shop.UpdateShop(currency, deck)) {
        if (waves.currentWave < (int)waves.waves.size() - 1)
            waves.StartNextWave();
        state = GameState::PLAYING;
    }
}

// ─── Input (PLAYING) ────────────────────────────────────

void Game::HandleInput() {
    if (IsKeyPressed(KEY_SPACE) && !waves.waveActive && waves.currentWave < (int)waves.waves.size()-1)
        waves.StartNextWave();

    // Hero Ultimate (Q key)
    if (IsKeyPressed(KEY_Q) && hero.IsUltReady()) {
        hero.FireUltimate();
        screenShakeTimer = 0.5f; // trigger screen shake
    }

    // Place tower
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && deck.HasSelection()) {
        Vector2 mp = GetMousePosition();
        bool clickedCard = false;
        for (int i = 0; i < (int)deck.hand.size(); i++)
            if (CheckCollisionPointRec(mp, GetHandSlotRect(i))) { clickedCard = true; break; }

        if (!clickedCard && mp.y < UI_PANEL_Y) {
            int col = (int)(mp.x/CELL_SIZE), row = (int)(mp.y/CELL_SIZE);
            if (col>=0 && col<GRID_COLS && row>=0 && row<GRID_ROWS) {
                Card* card = deck.GetSelectedCard();
                if (card) {
                    int cost = card->GetPlacementCost();
                    if (currency >= cost && grid[row][col].CanPlaceTower(card->def.baseTier)) {
                        Tower t(card->def.towerType, card->def.baseTier, grid[row][col].worldPos);
                        grid[row][col].PlaceTower(t);
                        currency -= cost;
                    }
                }
            }
        }
    }

    // Right-click: sell on grid, deselect otherwise
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        Vector2 mp = GetMousePosition();
        bool handled = false;

        if (mp.y < UI_PANEL_Y) {
            int col=(int)(mp.x/CELL_SIZE), row=(int)(mp.y/CELL_SIZE);
            if (col>=0&&col<GRID_COLS&&row>=0&&row<GRID_ROWS && !grid[row][col].IsEmpty()) {
                currency += grid[row][col].SellTopTower();
                handled = true;
            }
        }
        if (!handled) deck.DeselectAll();
    }

    // U key: upgrade top tower at hovered grid cell
    if (IsKeyPressed(KEY_U)) {
        Vector2 mp = GetMousePosition();
        if (mp.y < UI_PANEL_Y) {
            int col = (int)(mp.x/CELL_SIZE), row = (int)(mp.y/CELL_SIZE);
            if (col>=0 && col<GRID_COLS && row>=0 && row<GRID_ROWS) {
                grid[row][col].UpgradeTopTower(currency);
            }
        }
    }
}

void Game::UpdateProjectiles(float dt) {
    for (auto& p : projectiles) {
        p.Update(dt);
        for (auto& e : enemies)
            if (p.CheckCollision(e)) { if (!e.alive) currency += e.reward; break; }
    }
}

void Game::CheckEnemyReachedBase() {
    for (auto& e : enemies) {
        if (e.alive && e.ReachedEnd(pathPoints)) {
            hero.TakeDamage(1);
            e.alive = false;
            if (!hero.IsAlive()) state = GameState::GAME_OVER;
        }
    }
}

void Game::CleanupDead() {
    enemies.erase(std::remove_if(enemies.begin(),enemies.end(),[](const Enemy& e){return !e.alive;}),enemies.end());
    projectiles.erase(std::remove_if(projectiles.begin(),projectiles.end(),[](const Projectile& p){return !p.active;}),projectiles.end());
}

// ─── Ultimate Laser Visual ──────────────────────────────

void Game::DrawUltLaser() const {
    if (hero.ultActiveTimer <= 0) return;

    float alpha = hero.ultActiveTimer / hero.ultDuration; // fades out over duration
    float t = (float)GetTime();

    // Thick glowing laser segments tracing the entire path
    for (int i = 0; i < (int)pathPoints.size()-1; i++) {
        // Core beam (white-hot center)
        DrawLineEx(pathPoints[i], pathPoints[i+1], 8.0f, Fade(WHITE, 0.9f * alpha));
        // Inner glow (gold)
        DrawLineEx(pathPoints[i], pathPoints[i+1], 16.0f, Fade(COLOR_CURRENCY, 0.6f * alpha));
        // Outer glow (pulsing)
        float pulse = 20.0f + 6.0f * sinf(t * 15.0f + i * 2.0f);
        DrawLineEx(pathPoints[i], pathPoints[i+1], pulse, Fade(COLOR_CURRENCY, 0.15f * alpha));
        // Halo
        DrawLineEx(pathPoints[i], pathPoints[i+1], pulse + 12.0f, Fade(COLOR_CURRENCY, 0.05f * alpha));
    }

    // Bright nodes at each waypoint
    for (auto& pt : pathPoints) {
        float r = 10.0f + 4.0f * sinf(t * 12.0f);
        DrawCircleV(pt, r, Fade(WHITE, 0.8f * alpha));
        DrawCircleV(pt, r + 6, Fade(COLOR_CURRENCY, 0.2f * alpha));
    }
}

// ─── Draw ───────────────────────────────────────────────

void Game::Draw() const {
    ClearBackground(COLOR_BG);

    if (state == GameState::DRAFTING) { DrawDrafting(); return; }

    // ── Begin Camera2D (everything that shakes) ──────────
    BeginMode2D(camera);

    // Stars
    for (int i = 0; i < 80; i++) {
        int sx=(i*137+31)%SCREEN_WIDTH, sy=(i*211+47)%GRID_HEIGHT;
        DrawPixel(sx, sy, Fade(WHITE, 0.2f+0.15f*sinf((float)GetTime()*0.5f+i*0.7f)));
    }

    DrawPath(); DrawGrid(); DrawPortal();

    // Hero replaces old DrawBase()
    hero.Draw();

    for (int r=0;r<GRID_ROWS;r++) for(int c=0;c<GRID_COLS;c++) grid[r][c].DrawAll();

    // Cell highlights
    if (deck.HasSelection()) {
        Card* card = const_cast<Game*>(this)->deck.GetSelectedCard();
        if (card) {
            Vector2 mp = GetMousePosition();
            int hC=(int)(mp.x/CELL_SIZE), hR=(int)(mp.y/CELL_SIZE);
            for (int r=0;r<GRID_ROWS;r++) for(int c=0;c<GRID_COLS;c++) {
                if (grid[r][c].isPathTile) continue;
                bool ok = grid[r][c].CanPlaceTower(card->def.baseTier) && currency>=card->GetPlacementCost();
                if (r==hR&&c==hC&&mp.y<UI_PANEL_Y)
                    DrawRectangle(c*CELL_SIZE,r*CELL_SIZE,CELL_SIZE,CELL_SIZE, ok?COLOR_HOVER_VALID:COLOR_HOVER_INVALID);
                else if (ok)
                    DrawRectangle(c*CELL_SIZE,r*CELL_SIZE,CELL_SIZE,CELL_SIZE, COLOR_VALID_CELL);
            }
        }
    }

    for (auto& e : enemies) e.Draw();
    for (auto& p : projectiles) p.Draw(const_cast<AssetManager*>(&assets));

    // Ultimate laser drawn on top of path/enemies
    DrawUltLaser();

    // ── Hover Tooltip ────────────────────────────────────
    {
        Vector2 mp = GetMousePosition();
        if (mp.y < UI_PANEL_Y && mp.y >= 0) {
            int hC = (int)(mp.x/CELL_SIZE), hR = (int)(mp.y/CELL_SIZE);
            if (hC>=0 && hC<GRID_COLS && hR>=0 && hR<GRID_ROWS && !grid[hR][hC].IsEmpty()) {
                const GridNode& node = grid[hR][hC];
                const Tower& top = node.towerStack.back();
                float tx = mp.x + 16, ty = mp.y - 80;
                if (tx + 170 > SCREEN_WIDTH) tx = mp.x - 180;
                if (ty < 0) ty = mp.y + 20;

                DrawRectangle((int)tx-4, (int)ty-4, 178, 88, Fade(BLACK, 0.85f));
                DrawRectangleLinesEx({tx-4, ty-4, 178, 88}, 1, GetTierAccent(top.baseTier));

                char buf[64];
                snprintf(buf, sizeof(buf), "%s T%d Lv%d", GetTowerName(top.type), top.baseTier, top.fieldLevel);
                DrawText(buf, (int)tx, (int)ty, 12, COLOR_TEXT_MAIN);

                snprintf(buf, sizeof(buf), "DMG:%.0f  RNG:%.0f  SPD:%.1f",
                         top.effectiveStats.damage, top.effectiveStats.range, top.effectiveStats.fireRate);
                DrawText(buf, (int)tx, (int)(ty+16), 10, COLOR_TEXT_DIM);

                snprintf(buf, sizeof(buf), "Synergy: x%.2f", top.synergyMultiplier);
                DrawText(buf, (int)tx, (int)(ty+30), 10, Fade(COLOR_CARD_SEL, 0.9f));

                snprintf(buf, sizeof(buf), "Stack: %d tower(s)", (int)node.towerStack.size());
                DrawText(buf, (int)tx, (int)(ty+44), 10, COLOR_TEXT_DIM);

                if (top.CanUpgrade()) {
                    snprintf(buf, sizeof(buf), "[U] Upgrade: $%d", top.GetUpgradeCost());
                    float pulse = 0.6f + 0.4f * sinf((float)GetTime()*3.0f);
                    DrawText(buf, (int)tx, (int)(ty+60), 11, Fade(COLOR_CURRENCY, pulse));
                } else {
                    DrawText("MAX LEVEL", (int)tx, (int)(ty+60), 11, Fade(GetTierAccent(top.baseTier), 0.7f));
                }
            }
        }
    }

    EndMode2D();
    // ── End Camera2D ─────────────────────────────────────

    // UI is drawn OUTSIDE Camera2D so it stays static during shake
    DrawRectangle(0, UI_PANEL_Y, SCREEN_WIDTH, UI_PANEL_HEIGHT, COLOR_UI_PANEL);
    DrawLineEx({0,(float)UI_PANEL_Y},{(float)SCREEN_WIDTH,(float)UI_PANEL_Y}, 2, COLOR_UI_BORDER);
    DrawUI();

    // Shop overlay (drawn on top of everything)
    if (state == GameState::SHOP) DrawShop();

    if (state == GameState::GAME_OVER) DrawGameOver();
    if (state == GameState::VICTORY)   DrawVictory();
}

void Game::DrawDrafting() const {
    ClearBackground(COLOR_DRAFT_BG);
    deck.DrawDrafting(const_cast<AssetManager*>(&assets));

    for (int i = 0; i < (int)pathPoints.size()-1; i++)
        DrawLineEx(pathPoints[i], pathPoints[i+1], 2, Fade(COLOR_PATH_BORDER, 0.15f));
}

void Game::DrawShop() const {
    shop.DrawShop(currency, deck);
}

void Game::DrawGrid() const {
    for (int r=0;r<=GRID_ROWS;r++) DrawLine(0,r*CELL_SIZE,SCREEN_WIDTH,r*CELL_SIZE,COLOR_GRID_LINE);
    for (int c=0;c<=GRID_COLS;c++) DrawLine(c*CELL_SIZE,0,c*CELL_SIZE,GRID_HEIGHT,COLOR_GRID_LINE);
}

void Game::DrawPath() const {
    for (int r=0;r<GRID_ROWS;r++) for(int c=0;c<GRID_COLS;c++)
        if (pathCells[r][c]) DrawRectangle(c*CELL_SIZE,r*CELL_SIZE,CELL_SIZE,CELL_SIZE,COLOR_PATH_FILL);
    for (int i=0;i<(int)pathPoints.size()-1;i++) {
        DrawLineEx(pathPoints[i],pathPoints[i+1],4,COLOR_PATH_BORDER);
        DrawLineEx(pathPoints[i],pathPoints[i+1],12,COLOR_PATH_GLOW);
    }
    for (auto& pt : pathPoints) DrawCircleV(pt,4,COLOR_PATH_BORDER);
}

void Game::DrawUI() const {
    char hbuf[32]; snprintf(hbuf,sizeof(hbuf),"HP: %d/%d", hero.currentHP, hero.maxHP);
    DrawText(hbuf,20,UI_PANEL_Y+15,22,COLOR_HEALTH_BAR);
    float hpR=(float)hero.currentHP/hero.maxHP;
    DrawRectangle(20,UI_PANEL_Y+45,120,8,CLITERAL(Color){40,40,40,200});
    DrawRectangle(20,UI_PANEL_Y+45,(int)(120*hpR),8,COLOR_HEALTH_BAR);
    char cbuf[32]; snprintf(cbuf,sizeof(cbuf),"$ %d",currency);
    DrawText(cbuf,20,UI_PANEL_Y+65,22,COLOR_CURRENCY);
    deck.DrawPlaying(const_cast<AssetManager*>(&assets));
    waves.Draw();

    // Ult cooldown in UI
    if (hero.IsUltReady()) {
        float pulse = 0.6f + 0.4f * sinf((float)GetTime()*3.0f);
        DrawText("[Q] ULTIMATE READY", SCREEN_WIDTH-250, UI_PANEL_Y+75, 14,
                 Fade(COLOR_CURRENCY, pulse));
    } else if (hero.IsUltActive()) {
        DrawText("ULT ACTIVE!", SCREEN_WIDTH-200, UI_PANEL_Y+75, 14,
                 Fade(COLOR_CURRENCY, 0.9f));
    } else {
        float pct = hero.GetUltCooldownPercent();
        DrawRectangle(SCREEN_WIDTH-250, UI_PANEL_Y+78, 120, 8, CLITERAL(Color){40,40,40,200});
        DrawRectangle(SCREEN_WIDTH-250, UI_PANEL_Y+78, (int)(120*(1.0f-pct)), 8,
                      Fade(COLOR_CURRENCY, 0.5f));
        char ubuf[32]; snprintf(ubuf, sizeof(ubuf), "ULT: %.0f%%", (1.0f-pct)*100.0f);
        DrawText(ubuf, SCREEN_WIDTH-250, UI_PANEL_Y+90, 10, COLOR_TEXT_DIM);
    }

    DrawText("[1-5] Select  [LMB] Place  [RMB] Sell  [U] Upgrade  [Q] Ultimate  [F11] Fullscreen",
             20,UI_PANEL_Y+UI_PANEL_HEIGHT-18,10,COLOR_TEXT_DIM);
}

void Game::DrawPortal() const {
    Vector2 pp=pathPoints.front(); float p=1.0f+0.1f*sinf((float)GetTime()*3.0f);
    DrawCircleV(pp,24*p,Fade(COLOR_PORTAL,0.15f)); DrawCircleV(pp,16*p,Fade(COLOR_PORTAL,0.35f));
    DrawPoly(pp,3,12*p,270,COLOR_PORTAL);
    DrawText("SPAWN",(int)(pp.x-20),(int)(pp.y+22),10,Fade(COLOR_PORTAL,0.8f));
}

void Game::DrawGameOver() const {
    DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,Fade(BLACK,0.7f));
    const char* t="GAME OVER"; DrawText(t,(SCREEN_WIDTH-MeasureText(t,60))/2,SCREEN_HEIGHT/2-40,60,COLOR_HEALTH_BAR);
    const char* s="Press [R] to Restart"; DrawText(s,(SCREEN_WIDTH-MeasureText(s,20))/2,SCREEN_HEIGHT/2+30,20,COLOR_TEXT_DIM);
}

void Game::DrawVictory() const {
    DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,Fade(BLACK,0.7f));
    const char* t="VICTORY!"; DrawText(t,(SCREEN_WIDTH-MeasureText(t,60))/2,SCREEN_HEIGHT/2-40,60,COLOR_CARD_SEL);
    const char* s="All alien waves defeated! [R] Restart"; DrawText(s,(SCREEN_WIDTH-MeasureText(s,20))/2,SCREEN_HEIGHT/2+30,20,COLOR_TEXT_DIM);
}
