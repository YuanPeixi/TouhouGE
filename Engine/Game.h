#pragma once
#include "Interfaces.h"
#include "Math.h"
#include <vector>

enum class BulletType {
    StraightFixed,
    SinWaveFixed,
    AimedAtCreation,
    HomingRealtime
};

struct Bullet {
    Vec2 pos;
    Vec2 vel;
    float speed = 0.0f;
    float radius = 4.0f;
    Color color = Color::Cyan();
    bool fromPlayer = false;
    BulletType type = BulletType::StraightFixed;

    // SinWave
    Vec2  baseDir = Vec2(0.0f, 1.0f);
    float sinAmp = 40.0f;
    float sinFreq = 2.0f;
    float t = 0.0f;

    // Homing
    float turnRateRad = 2.0f;

    bool alive = true;
};

struct Entity {
    Vec2 pos;
    float radius = 14.0f;
    int   hp = 5;
    ITexture* texture = nullptr;
    float drawScale = 1.0f;
};

struct GameConfig {
    int width = 800;   // 逻辑宽度
    int height = 600;  // 逻辑高度
    float playerSpeed = 240.0f;
    float playerSlowMul = 0.45f;
    float enemySpeed = 40.0f;
    int   playerInitHP = 5;
    int   enemyInitHP = 50;
};

class GameEngine {
public:
    explicit GameEngine(const GameConfig& cfg);
    void SetTextures(ITexture* playerTex, ITexture* enemyTex);
    void Update(float dt, const IInput& input);
    void Render(IDrawContext& dc);

private:
    GameConfig cfg_;
    Entity player_;
    Entity enemy_;
    std::vector<Bullet> bullets_;
    float fireCooldown_ = 0.0f;
    float enemyPatternTimer_ = 0.0f;
    float enemyMoveTimer_ = 0.0f;
    bool gameOver_ = false;
    bool win_ = false;

    // 玩家受击后无敌时间（秒）
    static constexpr float kInvulnSeconds = 3.0f;
    float playerInvulnTimer_ = 0.0f;

    void ClampToBounds(Vec2& p, float margin=0.0f);
    void SpawnPlayerBullets();
    void SpawnEnemyPattern(float dt);
    void UpdateBullets(float dt, const IInput& input);
    void CheckCollisions();
    void KillOutside(Bullet& b);
};