#include "Game.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdlib>

Game::Game()
    : state(GameState::MAIN_MENU), currency(STARTING_CURRENCY),
      playerHealth(hero.currentHP), camera({0}), screenShakeTimer(0)
{
    memset(pathCells, false, sizeof(pathCells));
}

void Game::Init() {
    if (!IsWindowReady()) {
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ALIEN TD - Deck Builder Tower Defense");
        SetTargetFPS(TARGET_FPS);
    }

    // Bangun jalur
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

    // Inisialisasi kamera
    camera.offset = {0, 0};
    camera.target = {0, 0};
    camera.rotation = 0;
    camera.zoom = 1.0f;
    screenShakeTimer = 0;

    // ── Muat Aset (harus setelah window init) ──────────
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
        playlist = { "assets/bg/1.MP3", "assets/bg/2.MP3", "assets/bg/3.MP3", "assets/bg/4.MP3", "assets/bg/5.MP3" };
        currentTrackIndex = 0;
        bgMusic = LoadMusicStream(playlist[currentTrackIndex].c_str());
        PlayMusicStream(bgMusic);
        skipBtn = { (float)GetScreenWidth() - 350, (float)GetScreenHeight() - 60, 60, 30 };
    }
    assets.Load("proj_laser", "assets/laser_partikel.png");
    assets.Load("proj_missile", "assets/missiles_partikel.png");
    assets.Load("proj_freeze", "assets/frezze_partikel.png");
    assets.Load("proj_tesla", "assets/Tesla_Partikel.png");
    assets.Load("proj_plasma", "assets/Plasma_Partikel.png");
    // ── Sprite kartu (5 tipe × 3 tier) ─────────────────
    assets.Load("card_laser_t1",   "assets/LaserT1Card.png");
    assets.Load("card_laser_t2",   "assets/LaserT2Card.png");
    assets.Load("card_laser_t3",   "assets/LaserT3Card.png");
    assets.Load("card_missile_t1", "assets/MissileT1Card.png");
    assets.Load("card_missile_t2", "assets/MissileT2Card.png");
    assets.Load("card_missile_t3", "assets/MissileT3Card.png");
    assets.Load("card_freeze_t1",  "assets/FreezeT1Card.png");
    assets.Load("card_freeze_t2",  "assets/FreezeT2Card.png");
    assets.Load("card_freeze_t3",  "assets/FreezeT3Card.png");
    assets.Load("card_tesla_t1",   "assets/TeslaT1Card.png");
    assets.Load("card_tesla_t2",   "assets/TeslaT2Card.png");
    assets.Load("card_tesla_t3",   "assets/TeslaT3Card.png");
    assets.Load("card_plasma_t1",  "assets/PlasmaT1Card.png");
    assets.Load("card_plasma_t2",  "assets/PlasmaT2Card.png");
    assets.Load("card_plasma_t3",  "assets/PlasmaT3Card.png");
    // ── Sprite tower (sprite sheet berarah) ────────
    // FREEZE: T1-T3, keempat arah
    assets.Load("tower_freeze_1_f", "assets/FREEZE T1 F.png");
    assets.Load("tower_freeze_1_b", "assets/FREEZE T1 B.png");
    assets.Load("tower_freeze_1_l", "assets/FREEZE T1 L.png");
    assets.Load("tower_freeze_1_r", "assets/FREEZE T1 R.png");
    assets.Load("tower_freeze_2_f", "assets/FREEZE T2 F.png");
    assets.Load("tower_freeze_2_b", "assets/FREEZE T2 B.png");
    assets.Load("tower_freeze_2_l", "assets/FREEZE T2 L.png");
    assets.Load("tower_freeze_2_r", "assets/FREEZE T2 R.png");
    assets.Load("tower_freeze_3_f", "assets/FREEZE T3 F.png");
    assets.Load("tower_freeze_3_b", "assets/FREEZE T3 B.png");
    assets.Load("tower_freeze_3_l", "assets/FREEZE T3 L.png");
    assets.Load("tower_freeze_3_r", "assets/FREEZE T3 R.png");
    // LASER: T1-T3, keempat arah
    assets.Load("tower_laser_1_f", "assets/Laser T1 F.png");
    assets.Load("tower_laser_1_b", "assets/Laser T1 B.png");
    assets.Load("tower_laser_1_l", "assets/Laser T1 L.png");
    assets.Load("tower_laser_1_r", "assets/Laser T1 R.png");
    assets.Load("tower_laser_2_f", "assets/Laser T2 F.png");
    assets.Load("tower_laser_2_b", "assets/Laser T2 B.png");
    assets.Load("tower_laser_2_l", "assets/Laser T2 L.png");
    assets.Load("tower_laser_2_r", "assets/Laser T2 R.png");
    assets.Load("tower_laser_3_f", "assets/Laser T3 F.png");
    assets.Load("tower_laser_3_b", "assets/Laser T3 B.png");
    assets.Load("tower_laser_3_l", "assets/Laser T3 L.png");
    assets.Load("tower_laser_3_r", "assets/Laser T3 R.png");
    // MISSILE: T1-T3, keempat arah
    assets.Load("tower_missile_1_f", "assets/MISSILE T1 F.png");
    assets.Load("tower_missile_1_b", "assets/MISSILE T1 B.png");
    assets.Load("tower_missile_1_l", "assets/MISSILE T1 L.png");
    assets.Load("tower_missile_1_r", "assets/MISSILE T1 R.png");
    assets.Load("tower_missile_2_f", "assets/MISSILE T2 F.png");
    assets.Load("tower_missile_2_b", "assets/MISSILE T2 B.png");
    assets.Load("tower_missile_2_l", "assets/MISSILE T2 L.png");
    assets.Load("tower_missile_2_r", "assets/MISSILE T2 R.png");
    assets.Load("tower_missile_3_f", "assets/MISSILE T3 F.png");
    assets.Load("tower_missile_3_b", "assets/MISSILE T3 B.png");
    assets.Load("tower_missile_3_l", "assets/MISSILE T3 L.png");
    assets.Load("tower_missile_3_r", "assets/MISSILE T3 R.png");
    // PLASMA: T1 punya 4 arah, T2/T3 base saja
    assets.Load("tower_plasma_1_f", "assets/PLASMA T1 F.png");
    assets.Load("tower_plasma_1_b", "assets/PLASMA T1 B.png");
    assets.Load("tower_plasma_1_l", "assets/PLASMA T1 L.png");
    assets.Load("tower_plasma_1_r", "assets/PLASMA T1 R.png");
    assets.Load("tower_plasma_2",   "assets/Plasma T2.png");
    assets.Load("tower_plasma_3",   "assets/Plasma T3.png");
    // TESLA: base saja (tanpa varian berarah)
    assets.Load("tower_tesla_1",    "assets/Tesla T1.png");
    assets.Load("tower_tesla_2",    "assets/Tesla T2.png");
    assets.Load("tower_tesla_3",    "assets/Tesla T3.png");
    menuGifImage = LoadImageAnim("assets/Main Screen.gif", &menuAnimFrames);
    menuGifTexture = LoadTextureFromImage(menuGifImage);
    assets.Load("hero_marine", "assets/HeroMarine.png");
    assets.Load("ult_lightning", "assets/Lightning Strike.png");
    assets.Load("map_bg", "assets/Map.png");
    AssetManager::LoadSoundAsset("sfx_laser", "assets/laser_sound.mp3");
    AssetManager::LoadSoundAsset("sfx_missile", "assets/missile_sound.mp3");
    AssetManager::LoadSoundAsset("sfx_freeze", "assets/freeze_sound.wav");
    AssetManager::LoadSoundAsset("sfx_tesla", "assets/tesla_sound.MP3");
    AssetManager::LoadSoundAsset("sfx_plasma", "assets/plasma_sound.mp3");
    AssetManager::LoadSoundAsset("sfx_ult", "assets/ult_sound.mp3");
    // ── Sprite sheet musuh ──────────────────────────────
    assets.Load("enemy_boss",  "assets/alien_boss.png");
    assets.Load("enemy_fast",  "assets/alien_fast.png");
    assets.Load("enemy_grunt", "assets/alien_grunt.png");
    assets.Load("enemy_tank",  "assets/alien_tank.png");

    state = GameState::MAIN_MENU;
    currency = STARTING_CURRENCY;
    enemies.clear();
    projectiles.clear();
}

void Game::Shutdown() {
    assets.UnloadAll();
    UnloadTexture(menuGifTexture);
    UnloadImage(menuGifImage);
    if (IsAudioDeviceReady()) {
        UnloadMusicStream(bgMusic);
        CloseAudioDevice();
    }
}

void Game::InitGrid() {
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            grid[r][c] = GridNode(r, c, pathCells[r][c]);
}

void Game::PlayNextTrack() {
    StopMusicStream(bgMusic);
    UnloadMusicStream(bgMusic);
    currentTrackIndex = (currentTrackIndex + 1) % playlist.size();
    bgMusic = LoadMusicStream(playlist[currentTrackIndex].c_str());
    PlayMusicStream(bgMusic);
}

// ─── Update ─────────────────────────────────────────────

void Game::Update(float dt, const InputState& input) {
    lastInput = input;

    // Tarik alpha crosshair menuju ada/tidak tiap frame. Ini diletakkan
    // sebelum early-return berbasis state di bawah agar kursor fade dengan benar
    // di semua layar (menu, playing, shop, ...). fminf membatasi langkah agar
    // frame yang panjang tidak melampaui target.
    float fadeTarget = input.handPresent ? 1.0f : 0.0f;
    crosshairAlpha_ += (fadeTarget - crosshairAlpha_) * fminf(1.0f, 10.0f * dt);

    if (IsAudioDeviceReady()) {
        UpdateMusicStream(bgMusic);
        if (GetMusicTimePlayed(bgMusic) >= GetMusicTimeLength(bgMusic)) {
            PlayNextTrack();
        }
        if (input.clickPressed && CheckCollisionPointRec(input.cursor, skipBtn)) {
            PlayNextTrack();
        }
    }

    if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

    // ── Timer background map beranimasi ──────────────────
    mapAnimTimer += dt;
    if (mapAnimTimer >= MAP_FRAME_TIME) {
        mapAnimTimer = 0.0f;
        currentMapFrame = (currentMapFrame + 1) % MAP_TOTAL_FRAMES;
    }

    if (state == GameState::MAIN_MENU) {
        menuFrameTimer += dt;
        if (menuFrameTimer >= menuFrameDelay) {
            menuFrameTimer = 0.0f;
            menuCurrentFrame = (menuCurrentFrame + 1) % menuAnimFrames;

            // Hitung offset memori untuk frame berikutnya lalu kirim ke GPU
            int nextFrameDataOffset = menuGifImage.width * menuGifImage.height * 4 * menuCurrentFrame;
            UpdateTexture(menuGifTexture, ((unsigned char *)menuGifImage.data) + nextFrameDataOffset);
        }

        if (input.clickPressed) {
            state = GameState::DRAFTING;
        }
        return;
    }

    if (state == GameState::GAME_OVER || state == GameState::VICTORY) {
        if (IsKeyPressed(KEY_R)) Init();
        return;
    }

    hero.Update(dt, state == GameState::PLAYING && waves.waveActive);

    if (state == GameState::DRAFTING) { UpdateDrafting(input); return; }
    if (state == GameState::SHOP)    { UpdateShop(input); return; }

    // State PLAYING
    deck.UpdatePlaying(input);
    HandleInput(input);
    waves.Update(dt, enemies, pathPoints);

    for (int r=0;r<GRID_ROWS;r++)
        for (int c=0;c<GRID_COLS;c++)
            grid[r][c].UpdateAll(dt, enemies, projectiles);

    for (auto& e : enemies) e.Update(dt, pathPoints);
    UpdateProjectiles(dt);
    CheckEnemyReachedBase();
    CleanupDead();

    // ── Ultimate Hero: damage besar satu frame ──────────
    if (hero.isUltFiring) {
        for (auto& e : enemies) {
            if (!e.alive) continue;
            e.hp -= hero.ultDamage;
            if (e.hp <= 0) {
                e.alive = false;
                currency += e.reward;
                // Kill dari ult TIDAK memberi charge (cegah loop tak henti)
            }
        }
        hero.isUltFiring = false; // damage hanya di frame pertama
    }

    // ── Timer screen shake ───────────────────────────────
    if (screenShakeTimer > 0) {
        screenShakeTimer -= dt;
        float intensity = screenShakeTimer * 20.0f; // meluruh dari 10 ke 0
        if (intensity > 10.0f) intensity = 10.0f;
        camera.offset.x = (float)((rand() % 200 - 100) / 100.0f) * intensity;
        camera.offset.y = (float)((rand() % 200 - 100) / 100.0f) * intensity;
    } else {
        camera.offset.x = 0;
        camera.offset.y = 0;
    }

    // ── Cek shop akhir wave ──────────────────────────────
    CheckWaveEndShop();

    if (waves.AllWavesComplete() && waves.IsWaveCleared(enemies))
        state = GameState::VICTORY;
}

void Game::UpdateDrafting(const InputState& in) {
    deck.UpdateDrafting(in);

    if (deck.IsDraftReady() && in.clickPressed) {
        float btnW=220, btnH=50, btnX=(SCREEN_WIDTH-btnW)/2, btnY=SCREEN_HEIGHT-100;
        if (CheckCollisionPointRec(in.cursor, {btnX,btnY,btnW,btnH})) {
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

void Game::UpdateShop(const InputState& in) {
    if (shop.UpdateShop(currency, deck, in)) {
        if (waves.currentWave < (int)waves.waves.size() - 1)
            waves.StartNextWave();
        state = GameState::PLAYING;
    }
}

// ─── Input (PLAYING) ────────────────────────────────────

void Game::HandleInput(const InputState& in) {
    // Mulai wave berikutnya: SPACE ATAU gestur peace-sign tangan kanan
    if ((in.startWavePressed || IsKeyPressed(KEY_SPACE))
        && !waves.waveActive && waves.currentWave < (int)waves.waves.size()-1)
        waves.StartNextWave();

    // Ultimate Hero: tombol Q ATAU kepalan tangan kanan
    if ((in.ultPressed || IsKeyPressed(KEY_Q)) && hero.IsUltReady()) {
        hero.FireUltimate();
        screenShakeTimer = 0.5f; // picu screen shake
    }

    const Vector2 mp = in.cursor;

    // Pasang tower
    if (in.clickPressed && deck.HasSelection()) {
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

    // Jual tower (atau batal pilih): klik-kanan hardware ATAU gestur telapak terbuka tangan kiri
    if (in.sellPressed || in.rightClickPressed) {
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

    // Upgrade tower teratas di sel grid yang di-hover: tombol U ATAU gestur telunjuk tangan kiri
    if (in.upgradePressed || IsKeyPressed(KEY_U)) {
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
        for (auto& e : enemies) {
            if (p.CheckCollision(e)) {
                if (!e.alive) {
                    currency += e.reward;
                    // Beri charge ult berdasarkan tipe musuh
                    switch (e.type) {
                        case EnemyType::GRUNT: hero.AddUltCharge(2);  break;
                        case EnemyType::FAST:  hero.AddUltCharge(3);  break;
                        case EnemyType::TANK:  hero.AddUltCharge(10); break;
                        case EnemyType::BOSS:  hero.AddUltCharge(50); break;
                    }
                }
                break;
            }
        }
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

// ─── Visual Laser Ultimate ──────────────────────────────

void Game::DrawUltLaser() const {
    if (hero.ultActiveTimer <= 0) return;

    float alpha = hero.ultActiveTimer / hero.ultDuration; // memudar sepanjang durasi

    // ── Coba lightning berbasis sprite ────────────────────
    Texture2D* ltex = const_cast<AssetManager*>(&assets)->Get("ult_lightning");
    if (ltex && ltex->id > 0) {
        int frameSize = 64; // tiap frame 64x64 di sprite strip
        Rectangle src = {
            (float)(hero.currentUltFrame * frameSize), 0,
            (float)frameSize, (float)frameSize
        };

        // Gambar beam lightning di tiap segmen jalur
        for (int i = 0; i < (int)pathPoints.size()-1; i++) {
            Vector2 a = pathPoints[i];
            Vector2 b = pathPoints[i+1];
            float dx = b.x - a.x, dy = b.y - a.y;
            float segLen = sqrtf(dx*dx + dy*dy);
            float angle = atan2f(dy, dx) * RAD2DEG;

            // Regangkan sprite 64px agar menutupi panjang penuh segmen
            // Lebar = panjang segmen, Tinggi = ketebalan beam (64px)
            Rectangle dst = {
                a.x, a.y,
                segLen, (float)frameSize
            };
            // Origin di kiri-tengah agar beam mulai di titik A
            Vector2 origin = { 0, frameSize / 2.0f };

            DrawTexturePro(*ltex, src, dst, origin, angle, Fade(WHITE, alpha));

            // Layer glow tambahan (sedikit lebih besar, lebih transparan)
            Rectangle dstGlow = {
                a.x, a.y,
                segLen, (float)(frameSize + 16)
            };
            Vector2 originGlow = { 0, (frameSize + 16) / 2.0f };
            DrawTexturePro(*ltex, src, dstGlow, originGlow, angle,
                           Fade(COLOR_CURRENCY, 0.25f * alpha));
        }

        // Node terang di tiap waypoint
        float t = (float)GetTime();
        for (auto& pt : pathPoints) {
            float r = 10.0f + 4.0f * sinf(t * 12.0f);
            DrawCircleV(pt, r, Fade(WHITE, 0.6f * alpha));
        }
        return;
    }

    // ── Fallback prosedural ───────────────────────────────
    float t = (float)GetTime();
    for (int i = 0; i < (int)pathPoints.size()-1; i++) {
        DrawLineEx(pathPoints[i], pathPoints[i+1], 8.0f, Fade(WHITE, 0.9f * alpha));
        DrawLineEx(pathPoints[i], pathPoints[i+1], 16.0f, Fade(COLOR_CURRENCY, 0.6f * alpha));
        float pulse = 20.0f + 6.0f * sinf(t * 15.0f + i * 2.0f);
        DrawLineEx(pathPoints[i], pathPoints[i+1], pulse, Fade(COLOR_CURRENCY, 0.15f * alpha));
        DrawLineEx(pathPoints[i], pathPoints[i+1], pulse + 12.0f, Fade(COLOR_CURRENCY, 0.05f * alpha));
    }
    for (auto& pt : pathPoints) {
        float r = 10.0f + 4.0f * sinf(t * 12.0f);
        DrawCircleV(pt, r, Fade(WHITE, 0.8f * alpha));
        DrawCircleV(pt, r + 6, Fade(COLOR_CURRENCY, 0.2f * alpha));
    }
}

// ─── Draw ───────────────────────────────────────────────

void Game::Draw() const {
    ClearBackground(COLOR_BG);

    if (state == GameState::MAIN_MENU) {
        if (menuGifTexture.id > 0) {
            Rectangle sourceRec = { 0.0f, 0.0f, (float)menuGifTexture.width, (float)menuGifTexture.height };
            Rectangle destRec = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
            DrawTexturePro(menuGifTexture, sourceRec, destRec, {0.0f, 0.0f}, 0.0f, WHITE);
        }
        DrawCrosshair();
        return;
    }

    if (state == GameState::DRAFTING) { DrawDrafting(); DrawCrosshair(); return; }

    // ── Mulai Camera2D (semua yang bergetar) ──────────
    BeginMode2D(camera);

    // ── Background Map Beranimasi ─────────────────────────
    Texture2D* mapTex = const_cast<AssetManager*>(&assets)->Get("map_bg");
    if (mapTex && mapTex->id > 0) {
        Rectangle srcMap = { (float)(currentMapFrame * MAP_FRAME_W), 0.0f,
                             (float)MAP_FRAME_W, (float)MAP_FRAME_H };
        Rectangle dstMap = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
        DrawTexturePro(*mapTex, srcMap, dstMap, {0,0}, 0.0f, WHITE);
    } else {
        // Fallback prosedural (bintang)
        for (int i = 0; i < 80; i++) {
            int sx=(i*137+31)%SCREEN_WIDTH, sy=(i*211+47)%GRID_HEIGHT;
            DrawPixel(sx, sy, Fade(WHITE, 0.2f+0.15f*sinf((float)GetTime()*0.5f+i*0.7f)));
        }
    }

    DrawPath(); DrawGrid(); DrawPortal();

    // Hero menggantikan DrawBase() lama
    hero.Draw(const_cast<AssetManager*>(&assets));

    for (int r=0;r<GRID_ROWS;r++) for(int c=0;c<GRID_COLS;c++) grid[r][c].DrawAll();

    // Highlight sel
    if (deck.HasSelection()) {
        Card* card = const_cast<Game*>(this)->deck.GetSelectedCard();
        if (card) {
            Vector2 mp = lastInput.cursor;
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

    for (auto& e : enemies) e.Draw(const_cast<AssetManager*>(&assets));
    for (auto& p : projectiles) p.Draw(const_cast<AssetManager*>(&assets));

    // Laser ultimate digambar di atas jalur/musuh
    DrawUltLaser();

    // ── Tooltip Hover ────────────────────────────────────
    {
        Vector2 mp = lastInput.cursor;
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
    // ── Akhir Camera2D ─────────────────────────────────────

    // UI digambar DI LUAR Camera2D agar tetap statis saat shake
    DrawRectangle(0, UI_PANEL_Y, SCREEN_WIDTH, UI_PANEL_HEIGHT, COLOR_UI_PANEL);
    DrawLineEx({0,(float)UI_PANEL_Y},{(float)SCREEN_WIDTH,(float)UI_PANEL_Y}, 2, COLOR_UI_BORDER);
    DrawUI();

    // Overlay shop (digambar di atas segalanya)
    if (state == GameState::SHOP) DrawShop();

    if (state == GameState::GAME_OVER) DrawGameOver();
    if (state == GameState::VICTORY)   DrawVictory();

    DrawCrosshair();
}

void Game::DrawCrosshair() const {
    if (crosshairAlpha_ <= 0.01f) return;   // tangan keluar frame — disembunyikan penuh
    Vector2 c = lastInput.cursor;
    Color col = Fade(lastInput.udpAlive ? GREEN : RED, crosshairAlpha_);
    int cx = (int)c.x, cy = (int)c.y;
    DrawCircleLines(cx, cy, 14, col);
    DrawLine(cx - 18, cy, cx + 18, cy, col);
    DrawLine(cx, cy - 18, cx, cy + 18, col);
    if (lastInput.clickDown) DrawCircle(cx, cy, 6, col);
}

void Game::DrawDrafting() const {
    ClearBackground(COLOR_DRAFT_BG);
    deck.DrawDrafting(const_cast<AssetManager*>(&assets));

    for (int i = 0; i < (int)pathPoints.size()-1; i++)
        DrawLineEx(pathPoints[i], pathPoints[i+1], 2, Fade(COLOR_PATH_BORDER, 0.15f));
}

void Game::DrawShop() const {
    shop.DrawShop(currency, deck, const_cast<AssetManager*>(&assets));
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

    // ── UI Pemutar Musik ──────────────────────────────────
    DrawRectangleRec(skipBtn, CLITERAL(Color){30,30,30,255});
    DrawRectangleLinesEx(skipBtn, 1, COLOR_CURRENCY);
    DrawText("[ >> ]", skipBtn.x + 12, skipBtn.y + 8, 14, COLOR_CURRENCY);
    DrawText(TextFormat("Track: %d", currentTrackIndex + 1), skipBtn.x - 100, skipBtn.y + 5, 20, RAYWHITE);

    // Charge ult di UI (berbasis kill)
    if (hero.IsUltReady()) {
        float pulse = 0.6f + 0.4f * sinf((float)GetTime()*3.0f);
        DrawText("[Q] ULTIMATE READY", SCREEN_WIDTH-250, UI_PANEL_Y+75, 14,
                 Fade(COLOR_CURRENCY, pulse));
    } else if (hero.IsUltActive()) {
        DrawText("ULT ACTIVE!", SCREEN_WIDTH-200, UI_PANEL_Y+75, 14,
                 Fade(COLOR_CURRENCY, 0.9f));
    } else {
        float pct = hero.GetUltChargePercent();
        DrawRectangle(SCREEN_WIDTH-250, UI_PANEL_Y+78, 120, 8, CLITERAL(Color){40,40,40,200});
        DrawRectangle(SCREEN_WIDTH-250, UI_PANEL_Y+78, (int)(120*pct), 8,
                      Fade(COLOR_CURRENCY, 0.5f));
        char ubuf[32]; snprintf(ubuf, sizeof(ubuf), "ULT: %d/%d", hero.currentUltCharge, hero.maxUltCharge);
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
