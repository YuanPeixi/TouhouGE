# Interfaces and Contracts

This document lists the core interfaces, data structures, and public APIs with their semantics, to guide further development and porting.

## Coordinate, Units, and Timing

- Coordinates: logical pixels (default 800x600), origin at top-left, +X right, +Y down.
- Time step: seconds (float `dt`).
- Speeds: pixels per second.
- Radii: pixels (logical).

---

## Engine Public API

### GameEngine

| Method | Signature | Description | Notes |
|---|---|---|---|
| ctor | `GameEngine(const GameConfig& cfg)` | Initializes gameplay with logical size and parameters. | Player at 80% height; enemy near top. |
| SetTextures | `void SetTextures(ITexture* playerTex, ITexture* enemyTex)` | Assigns textures (owned externally). | Pass nullptr to use circle placeholders. |
| Update | `void Update(float dt, const IInput& input)` | Advances game state by `dt` using input. | Spawns enemy patterns if not game over, integrates bullets, handles collisions, invuln timer. |
| Render | `void Render(IDrawContext& dc)` | Renders entire frame in logical coordinates. | Uses HUD “★” and draws halo if invulnerable. |

### GameConfig

| Field | Type | Default | Meaning |
|---|---|---:|---|
| width | int | 800 | Logical width |
| height | int | 600 | Logical height |
| playerSpeed | float | 240 | Movement speed px/s |
| playerSlowMul | float | 0.45 | Speed multiplier when Shift held |
| enemySpeed | float | 40 | Reserved (simple float motion) |
| playerInitHP | int | 5 | Initial player HP |
| enemyInitHP | int | 50 | Initial enemy HP |

---

## Engine Data Types

### Entity

| Field | Type | Meaning |
|---|---|---|
| pos | Vec2 | Center position |
| radius | float | Collision circle radius |
| hp | int | Hit points |
| texture | ITexture* | Optional sprite |
| drawScale | float | Texture scale |

### Bullet

| Field | Type | Meaning |
|---|---|---|
| pos | Vec2 | Center position |
| vel | Vec2 | Current velocity vector |
| speed | float | Base speed magnitude |
| radius | float | Collision circle radius |
| color | Color | Fill color |
| fromPlayer | bool | True if fired by player |
| type | BulletType | Behavior type |
| baseDir | Vec2 | Unit direction (Sin wave forward) |
| sinAmp | float | Sin lateral amplitude (px) |
| sinFreq | float | Sin frequency (Hz) |
| t | float | Lifetime (s) used by Sin oscillation |
| turnRateRad | float | Max turn rate for homing (rad/s) |
| alive | bool | Active flag |

### BulletType

- StraightFixed: constant `vel`, linear motion.
- SinWaveFixed: advance along `baseDir`, sinusoidal displacement perpendicular to baseDir.
- AimedAtCreation: `vel` initialized toward player at spawn time; then linear.
- HomingRealtime: `vel`’s direction turns toward the player each frame, capped by `turnRateRad`.

### Vec2

- Basic 2D vector with arithmetic, `length()`, `normalized()`.

### Color

- RGBA 8-bit channels with static helpers (Black/White/Red/...).

---

## Engine Contracts and Behavior

- Bounds: Player position is clamped to `[radius, width-radius] x [radius, height-radius]`.
- Bullets are culled when leaving logical bounds by a margin (32 px).
- Collision: Circle-circle; on player hit:
  - If `playerInvulnTimer_ > 0`, ignore damage.
  - Else decrement `player.hp`, set `playerInvulnTimer_ = 3.0s`, and destroy the bullet.
- Win/Lose: Enemy HP reaches 0 → win; Player HP reaches 0 → game over.
- Firing: Player fires when Z held, with a cooldown (0.09 s), two straight bullets.

---

## Rendering Interface

### IDrawContext

| Method | Signature | Semantics |
|---|---|---|
| GetSize | `Size2D GetSize() const` | Returns logical size used by the engine. |
| Clear | `void Clear(const Color& c)` | Clears the entire target (logical). |
| DrawCircle | `void DrawCircle(float x, float y, float radius, const Color& fill, const Color& border, float borderWidth=1.0f)` | Filled circle with outline at center `(x,y)`. |
| DrawImage | `void DrawImage(ITexture* tex, float x, float y, float scale=1.0f)` | Draws the texture centered, scaled by `scale`. |
| DrawTextW | `void DrawTextW(const std::wstring& text, float x, float y, const Color& color, int alignFlags)` | UTF-16 text. Alignment flags: `AlignLeft/Right/Top/Bottom/HCenter/VCenter`. Coordinates are logical. |

### ITexture

| Method | Signature | Notes |
|---|---|---|
| Width | `int Width() const` | Texture pixel width |
| Height | `int Height() const` | Texture pixel height |

---

## Input Interface

### IInput

| Method | Signature | Semantics |
|---|---|---|
| IsKeyDown | `bool IsKeyDown(int vk) const` | Virtual-key code (Win32 VK_*) or ASCII ('A','Z',...). |
| IsMouseDown | `bool IsMouseDown(int button) const` | 0: Left, 1: Right, 2: Middle |
| MouseX | `float MouseX() const` | Logical X (if platform maps pixel→logical). |
| MouseY | `float MouseY() const` | Logical Y. |

Movement:
- Keyboard: Arrow/WASD, Shift for slow.
- Mouse: While right button is held, player moves toward the mouse at current speed.

---

## Platform (MFC/GDI+) Mapping

### MfcDrawContext (implements IDrawContext)

| Method | Signature | Semantics |
|---|---|---|
| ctor | `MfcDrawContext(CWnd* target, int clientW, int clientH)` | Creates back buffer sized to client pixels. |
| SetLogicalSize | `void SetLogicalSize(int logicalW, int logicalH)` | Sets logical space; computes scaleX/scaleY = client/logical. |
| Begin | `void Begin()` | Resets transform and applies scale (logical→pixel). |
| End | `void End(CDC* pDC)` | Blits back buffer to window DC. |
| Draw* | as per `IDrawContext` | Drawing occurs in scaled world-space. |
| GetSize | returns logical size | Used by engine HUD placement. |

### MfcInput (implements IInput)

| Method | Signature | Semantics |
|---|---|---|
| HandleKeyDown/Up | `(WPARAM vk)` | Updates `keys_`. |
| HandleMouseMove | `(LPARAM lParam)` | Stores raw pixel coordinates. |
| HandleMouseButton | `(int button, bool down)` | Updates mouse btn states. |
| SetMouseToLogicalScale | `(float sx, float sy)` | Sets pixel→logical scale factors (logical/client). |
| MouseX/MouseY | `float` | Returns `raw * scale` (logical coordinates). |

### CGameWnd

- Owns `GameEngine`, `MfcDrawContext`, `MfcInput`.
- WM_TIMER drives fixed-time rendering, high-resolution performance counters compute `dt`.
- OnPaint uses Begin → Render → End.

---

## Extension Points

- Add more bullet behaviors: introduce new `BulletType` and extend `UpdateBullets`.
- Add stage scripting: replace `SpawnEnemyPattern` with an emitter system (timelines, wave configs).
- Add more platforms: implement `IDrawContext` and `IInput` for SDL2, Direct2D, OpenGL, etc. Engine code remains unchanged.
- Dynamic resize: handle `WM_SIZE` to recreate back buffer and recompute scaling in `MfcDrawContext`, and call `SetMouseToLogicalScale` accordingly.
