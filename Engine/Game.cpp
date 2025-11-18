#include "Game.h"
#include <windows.h> // for VK_*
#include <algorithm>
#include <cmath>

GameEngine::GameEngine(const GameConfig& cfg) : cfg_(cfg) {
    player_.pos = Vec2(cfg.width * 0.5f, cfg.height * 0.80f);
    player_.radius = 6.0f;
    player_.hp = cfg.playerInitHP;
    player_.drawScale = 0.8f;

    enemy_.pos = Vec2(cfg.width * 0.5f, cfg.height * 0.18f);
    enemy_.radius = 18.0f;
    enemy_.hp = cfg.enemyInitHP;
    enemy_.drawScale = 1.0f;
}

void GameEngine::SetTextures(ITexture* playerTex, ITexture* enemyTex) {
    player_.texture = playerTex;
    enemy_.texture = enemyTex;
}

void GameEngine::ClampToBounds(Vec2& p, float margin) {
    p.x = Clamp(p.x, margin, (float)cfg_.width - margin);
    p.y = Clamp(p.y, margin, (float)cfg_.height - margin);
}

void GameEngine::SpawnPlayerBullets() {
    Bullet b1, b2;
    b1.fromPlayer = true; b2.fromPlayer = true;
    b1.type = BulletType::StraightFixed;
    b2.type = BulletType::StraightFixed;
    b1.color = Color::Yellow();
    b2.color = Color::Yellow();
    b1.radius = b2.radius = 4.0f;
    b1.speed = b2.speed = 480.0f;
    b1.vel = Vec2(0.0f, -b1.speed);
    b2.vel = Vec2(0.0f, -b2.speed);
    float off = 10.0f;
    b1.pos = Vec2(player_.pos.x - off, player_.pos.y - 10.0f);
    b2.pos = Vec2(player_.pos.x + off, player_.pos.y - 10.0f);
    bullets_.push_back(b1);
    bullets_.push_back(b2);
}

void GameEngine::SpawnEnemyPattern(float dt) {
    enemyPatternTimer_ += dt;

    const float interval = 0.25f;
    static int phase = 0;
    if (enemyPatternTimer_ >= interval) {
        enemyPatternTimer_ = 0.0f;
        phase = (phase + 1) % 4;

        switch (phase) {
        case 0: {
            int n = 18;
            float speed = 140.0f;
            for (int i=0;i<n;++i) {
                float a = (float)i / n * 6.2831853f;
                Bullet b;
                b.fromPlayer = false;
                b.type = BulletType::StraightFixed;
                b.color = Color::Magenta();
                b.radius = 5.0f;
                b.speed = speed;
                b.vel = Vec2(std::cos(a)*speed, std::sin(a)*speed);
                b.pos = enemy_.pos;
                bullets_.push_back(b);
            }
            break;
        }
        case 1: {
            int n = 10;
            for (int i=0;i<n;++i) {
                Bullet b;
                b.fromPlayer = false;
                b.type = BulletType::SinWaveFixed;
                b.color = Color::Blue();
                b.radius = 5.0f;
                b.speed = 120.0f;
                b.baseDir = Vec2(0.0f, 1.0f);
                b.sinAmp = 60.0f;
                b.sinFreq = 1.8f;
                b.pos = enemy_.pos + Vec2((float)(i - n/2) * 8.0f, 0.0f);
                bullets_.push_back(b);
            }
            break;
        }
        case 2: {
            int n = 8;
            float speed = 180.0f;
            for (int i=0;i<n;++i) {
                Bullet b;
                b.fromPlayer = false;
                b.type = BulletType::AimedAtCreation;
                b.color = Color::Red();
                b.radius = 6.0f;
                b.speed = speed;
                Vec2 toPlayer = (player_.pos - enemy_.pos).normalized();
                float offset = ((i - (n-1)/2.0f) / (float)n) * 0.5f; // radians
                float ca = std::cos(offset), sa = std::sin(offset);
                Vec2 dir = Vec2(toPlayer.x * ca - toPlayer.y * sa, toPlayer.x * sa + toPlayer.y * ca);
                b.vel = dir * speed;
                b.pos = enemy_.pos;
                bullets_.push_back(b);
            }
            break;
        }
        case 3: {
            int n = 6;
            for (int i=0;i<n;++i) {
                Bullet b;
                b.fromPlayer = false;
                b.type = BulletType::HomingRealtime;
                b.color = Color::Green();
                b.radius = 6.0f;
                b.speed = 160.0f;
                b.turnRateRad = 2.6f;
                b.vel = Vec2(0.0f, 1.0f) * b.speed;
                b.pos = enemy_.pos + Vec2((float)(i - n/2) * 10.0f, 0.0f);
                bullets_.push_back(b);
            }
            break;
        }
        }
    }

    enemyMoveTimer_ += dt;
    enemy_.pos.y = Lerp((float)cfg_.height * 0.14f, (float)cfg_.height * 0.22f, (std::sin(enemyMoveTimer_*1.2f)+1.0f)*0.5f);
}

void GameEngine::UpdateBullets(float dt, const IInput& /*input*/) {
    for (auto& b : bullets_) {
        if (!b.alive) continue;
        b.t += dt;
        switch (b.type) {
        case BulletType::StraightFixed:
        case BulletType::AimedAtCreation:
            b.pos += b.vel * dt;
            break;
        case BulletType::SinWaveFixed: {
            Vec2 dir = b.baseDir.normalized();
            Vec2 perp = Vec2(-dir.y, dir.x);
            Vec2 forward = dir * (b.speed * dt);
            float sideNow = std::sin(b.t * 6.2831853f * b.sinFreq) * b.sinAmp;
            float sidePrev = std::sin((b.t - dt) * 6.2831853f * b.sinFreq) * b.sinAmp;
            b.pos += forward + perp * (sideNow - sidePrev);
            break;
        }
        case BulletType::HomingRealtime: {
            Vec2 desiredDir = (player_.pos - b.pos).normalized();
            Vec2 curDir = b.vel.normalized();
            float curAng = std::atan2(curDir.y, curDir.x);
            float desAng = std::atan2(desiredDir.y, desiredDir.x);
            float delta = WrapAngle(desAng - curAng);
            float maxTurn = b.turnRateRad * dt;
            float turn = Clamp(delta, -maxTurn, maxTurn);
            float newAng = curAng + turn;
            Vec2 newDir = Vec2(std::cos(newAng), std::sin(newAng));
            b.vel = newDir * b.speed;
            b.pos += b.vel * dt;
            break;
        }
        }
        KillOutside(b);
    }
    bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(), [](const Bullet& b){ return !b.alive; }), bullets_.end());
}

void GameEngine::KillOutside(Bullet& b) {
    const float m = 32.0f;
    if (b.pos.x < -m || b.pos.x > cfg_.width + m || b.pos.y < -m || b.pos.y > cfg_.height + m) {
        b.alive = false;
    }
}

void GameEngine::CheckCollisions() {
    if (gameOver_ || win_) return;

    for (auto& b : bullets_) {
        if (!b.alive) continue;
        if (b.fromPlayer) {
            Vec2 d = b.pos - enemy_.pos;
            float dist2 = d.x*d.x + d.y*d.y;
            float rr = (b.radius + enemy_.radius) * (b.radius + enemy_.radius);
            if (dist2 <= rr) {
                b.alive = false;
                enemy_.hp -= 1;
                if (enemy_.hp <= 0) { enemy_.hp = 0; win_ = true; }
            }
        } else {
            // 敌弹击中玩家：若在无敌时间,忽略伤害（也不清除子弹）
            if (playerInvulnTimer_ > 0.0f) {
                continue;
            }
            Vec2 d = b.pos - player_.pos;
            float dist2 = d.x*d.x + d.y*d.y;
            float rr = (b.radius + player_.radius) * (b.radius + player_.radius);
            if (dist2 <= rr) {
                b.alive = false;
                player_.hp -= 1;
                playerInvulnTimer_ = kInvulnSeconds;
                if (player_.hp <= 0) { player_.hp = 0; gameOver_ = true; }
            }
        }
    }
}

void GameEngine::Update(float dt, const IInput& input) {
    if (!(gameOver_ || win_)) {
        SpawnEnemyPattern(dt);
    }

    // 无敌计时衰减
    if (playerInvulnTimer_ > 0.0f) {
        playerInvulnTimer_ -= dt;
        if (playerInvulnTimer_ < 0.0f) playerInvulnTimer_ = 0.0f;
    }

    float speed = cfg_.playerSpeed * (input.IsKeyDown(VK_SHIFT) ? cfg_.playerSlowMul : 1.0f);
    Vec2 v = Vec2(0.0f, 0.0f);
    if (input.IsKeyDown(VK_LEFT) || input.IsKeyDown('A')) v.x -= 1.f;
    if (input.IsKeyDown(VK_RIGHT)|| input.IsKeyDown('D')) v.x += 1.f;
    if (input.IsKeyDown(VK_UP)   || input.IsKeyDown('W')) v.y -= 1.f;
    if (input.IsKeyDown(VK_DOWN) || input.IsKeyDown('S')) v.y += 1.f;
    if (v.length() > 0.0f) v = v.normalized() * speed;
    player_.pos += v * dt;

    if (input.IsMouseDown(1)) {
        Vec2 mouse(input.MouseX(), input.MouseY());
        Vec2 to = mouse - player_.pos;
        float dist = to.length();
        if (dist > 1e-3f) {
            Vec2 step = to.normalized() * speed * dt;
            if (step.length() >= dist) player_.pos = mouse;
            else player_.pos += step;
        }
    }

    ClampToBounds(player_.pos, 6.0f);

    fireCooldown_ -= dt;
    if (fireCooldown_ < 0) fireCooldown_ = 0;
    if (input.IsKeyDown('Z') && !gameOver_ && !win_) {
        const float fireInterval = 0.09f;
        if (fireCooldown_ <= 0.0f) {
            SpawnPlayerBullets();
            fireCooldown_ = fireInterval;
        }
    }

    UpdateBullets(dt, input);
    CheckCollisions();
}

void GameEngine::Render(IDrawContext& dc) {
    dc.Clear(Color::Black());

    if (enemy_.texture) dc.DrawImage(enemy_.texture, enemy_.pos.x, enemy_.pos.y, enemy_.drawScale);
    else dc.DrawCircle(enemy_.pos.x, enemy_.pos.y, 20.0f, Color::Magenta(), Color::White(), 1.0f);

    // 玩家渲染：无敌时绘制轻微光环
    if (player_.texture) dc.DrawImage(player_.texture, player_.pos.x, player_.pos.y, player_.drawScale);
    else dc.DrawCircle(player_.pos.x, player_.pos.y, 12.0f, Color::Cyan(), Color::White(), 1.0f);
    dc.DrawCircle(player_.pos.x, player_.pos.y, player_.radius, Color{255,255,255,30}, Color::White(), 1.0f);

    if (playerInvulnTimer_ > 0.0f) {
        dc.DrawCircle(player_.pos.x, player_.pos.y, player_.radius + 8.0f, Color{120,200,255,40}, Color::White(), 1.0f);
    }

    for (const auto& b : bullets_) {
        if (!b.alive) continue;
        dc.DrawCircle(b.pos.x, b.pos.y, b.radius, b.color, Color::Black(), 1.0f);
    }

    std::wstring pLine = L"P ";
    pLine += L"\u2605";
    pLine += L" ";
    pLine += std::to_wstring(player_.hp);

    std::wstring eLine = L"E ";
    eLine += L"\u2605";
    eLine += L" ";
    eLine += std::to_wstring(enemy_.hp);

    const float W = (float)dc.GetSize().width;
    dc.DrawTextW(pLine, W - 12.0f, 12.0f, Color::White(), AlignRight | AlignTop);
    dc.DrawTextW(eLine, W - 12.0f, 36.0f, Color::White(), AlignRight | AlignTop);

    if (gameOver_) {
        dc.DrawTextW(L"GAME OVER", dc.GetSize().width * 0.5f, dc.GetSize().height * 0.5f, Color::Red(), AlignHCenter | AlignVCenter);
    } else if (win_) {
        dc.DrawTextW(L"YOU WIN", dc.GetSize().width * 0.5f, dc.GetSize().height * 0.5f, Color::Green(), AlignHCenter | AlignVCenter);
    }
}