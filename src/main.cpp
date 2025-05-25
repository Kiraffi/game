#include <cstdint>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include <raylib.h>
#include <raymath.h>

static const float ROTATION_SPEED = 4.0;

static const float PHYSICS_TIME_STEP = 1.0 / 64.0;
static const float SPEED = 256.0;
static const float BULLET_SPEED = 512.0;
static const float BULLET_MAX_TIME = 2.0;

static const float MAX_ROCK_SPEED = 128.0;


enum Team : uint8_t
{
    PlayerTeam,
    EnemyTeam,
    NeutralTeam, // maybe?
};

struct Player
{
    Vector2 pos;
    Vector2 speed;
    float rotation;
    float shootCD;
};

struct Bullet
{
    Vector2 pos;
    Vector2 speed;
    float time;
    Team team;
};

struct Rock
{
    Vector2 pos;
    Vector2 speed;
    float rotation;
};

struct Enemy
{

};

struct Cam
{
    Vector2 pos;

};

struct MyVars
{
    double dtOver;
    uint64_t physicsTimeStep;
    Player player;
};


MyVars g_vars;

std::vector<Bullet> bullets;
std::vector<Rock> rocks;

template <typename T>
static void listSwapRemove(T& vec, int index)
{
    std::swap(*(vec.begin() + index), *(vec.end() - 1));
    vec.erase(vec.end() - 1);
}

static void init();
static void deinit();
static void update();

static void updatePhysics();
static void updateShip(double dt);
static void updateBullets(double dt);
static void updateRocks(double dt);

static void render();
static void drawGrid(int32_t screenWidth, int32_t screenHeight);
static void drawPlayer(int32_t screenWidth, int32_t screenHeight);
static void drawBullets(int32_t screenWidth, int32_t screenHeight);
static void drawRocks(int32_t screenWidth, int32_t screenHeight);


static float randomFloat()
{
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

int main(void)
{
    InitWindow(1280, 720, "Game");
    init();

    while (!WindowShouldClose())
    {
        update();
        updatePhysics();
        render();
    }

    deinit();
    return 0;
}


static void init()
{
    g_vars.player = { .pos = {0, 0}, .speed = {0, 0}, .rotation = 0 };

    g_vars.physicsTimeStep = 0;
    g_vars.dtOver = 0.0;

}

static void deinit()
{
    CloseWindow();
}

static void update()
{
    //double dt = GetFrameTime();
}

static void updatePhysics()
{
    double dt = GetFrameTime();
    g_vars.dtOver += dt;

    int32_t physicsSteps = 0;

    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();

    float w = screenWidth;
    float h = screenHeight;

    while(g_vars.dtOver >= PHYSICS_TIME_STEP)
    {
        g_vars.dtOver -= PHYSICS_TIME_STEP;
        g_vars.physicsTimeStep += 1;
        physicsSteps += 1;

        updateShip(PHYSICS_TIME_STEP);
        updateBullets(PHYSICS_TIME_STEP);
        updateRocks(PHYSICS_TIME_STEP);
    }
    std::string str = "Pos: ";
    str += std::to_string(g_vars.player.pos.x);
    str += ", ";
    str += std::to_string(g_vars.player.pos.y);
    DrawText(str.c_str(), 10, 30, 16, RAYWHITE);
}

static void updateShip(double dt)
{
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();

    if(IsKeyDown(KEY_A)) g_vars.player.rotation -= dt * ROTATION_SPEED;
    if(IsKeyDown(KEY_D)) g_vars.player.rotation += dt * ROTATION_SPEED;

    while(g_vars.player.rotation >= 2.0 * PI) g_vars.player.rotation -= 2.0 * PI;
    while(g_vars.player.rotation <= 0.0 * PI) g_vars.player.rotation += 2.0 * PI;

    float c = std::cos(g_vars.player.rotation);
    float s = std::sin(g_vars.player.rotation);

    Vector2 forward = {c, s};

    g_vars.player.shootCD = std::max(0.0f, g_vars.player.shootCD - float(dt));

    if(IsKeyDown(KEY_W)) g_vars.player.speed +=  forward * dt * SPEED;
    if(IsKeyDown(KEY_S)) g_vars.player.speed -= forward * dt * SPEED;

    g_vars.player.speed *= 0.99;
    g_vars.player.pos += g_vars.player.speed * dt;

    while(g_vars.player.pos.x >= screenWidth * 0.5) g_vars.player.pos.x -= screenWidth;
    while(g_vars.player.pos.x <= -screenWidth * 0.5) g_vars.player.pos.x += screenWidth;
    while(g_vars.player.pos.y >= screenHeight * 0.5) g_vars.player.pos.y -= screenHeight;
    while(g_vars.player.pos.y <= -screenHeight * 0.5) g_vars.player.pos.y += screenHeight;
}

static void updateBullets(double dt)
{
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();

    float c = std::cos(g_vars.player.rotation);
    float s = std::sin(g_vars.player.rotation);
    Vector2 forward = {c, s};

    if(IsKeyDown(KEY_SPACE) && g_vars.player.shootCD <= 0.0)
    {
        g_vars.player.shootCD = 0.25;
        bullets.emplace_back(Bullet{
            .pos = g_vars.player.pos,
            .speed = forward * BULLET_SPEED + g_vars.player.speed,
            .time = 0.0,
            .team = PlayerTeam
        });
    }

    for(int i = bullets.size() - 1; i >= 0; --i)
    {
        Bullet& b = bullets[i];
        b.pos += b.speed * dt;
        b.time += dt;
        if(b.time >= BULLET_MAX_TIME)
        {
            listSwapRemove(bullets, i);
            continue;
        }
        while(b.pos.x >= screenWidth * 0.5)   b.pos.x -= screenWidth;
        while(b.pos.x <= -screenWidth * 0.5)  b.pos.x += screenWidth;
        while(b.pos.y >= screenHeight * 0.5)  b.pos.y -= screenHeight;
        while(b.pos.y <= -screenHeight * 0.5) b.pos.y += screenHeight;
    }
}
static void updateRocks(double dt)
{
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();

    if(rocks.size() < 10)
    {
        rocks.emplace_back(Rock{
            .pos = Vector2{screenWidth * randomFloat(), screenHeight * randomFloat()},
            .speed = Vector2{
                (randomFloat() * 2.0f - 1.0f),
                (randomFloat() * 2.0f - 1.0f)} * MAX_ROCK_SPEED
        });
    }

    for(int i = rocks.size() - 1; i >= 0; --i)
    {
        Rock& r = rocks[i];
        r.pos += r.speed * dt;
        while(r.pos.x >= screenWidth * 0.5)   r.pos.x -= screenWidth;
        while(r.pos.x <= -screenWidth * 0.5)  r.pos.x += screenWidth;
        while(r.pos.y >= screenHeight * 0.5)  r.pos.y -= screenHeight;
        while(r.pos.y <= -screenHeight * 0.5) r.pos.y += screenHeight;

        for(const Bullet& b : bullets)
        {
            Vector2 diff = r.pos - b.pos;
            float d = Vector2DotProduct(diff, diff);
            if(d < 125.0f)
            {
                listSwapRemove(rocks, i);
                break;
            }
        }

    }

}

void render()
{
    BeginDrawing();

    ClearBackground({0, 64, 96, 0});

    int32_t screenWidth = GetScreenWidth();
    int32_t screenHeight = GetScreenHeight();

    drawGrid(screenWidth, screenHeight);
    drawPlayer(screenWidth, screenHeight);
    drawBullets(screenWidth, screenHeight);
    drawRocks(screenWidth, screenHeight);

    DrawFPS(4, 4);

    EndDrawing();
}





void drawGrid(int32_t screenWidth, int32_t screenHeight)
{
    const static int32_t VerticalLines = 31;
    const static int32_t HorizontalLines = 31;

    for(int i = 1; i <= VerticalLines; ++i)
    {
        int32_t x = (screenWidth * i / (VerticalLines + 1));
        DrawLine(x, 0, x, screenHeight, BLACK);
    }

    for(int i = 1; i <= HorizontalLines; ++i)
    {
        int32_t y = (screenHeight * i / (HorizontalLines + 1));
        DrawLine(0, y, screenWidth, y, BLACK);
    }
}

void drawPlayer(int32_t screenWidth, int32_t screenHeight)
{
    float c = std::cos(g_vars.player.rotation);
    float s = std::sin(g_vars.player.rotation);

    float w = screenWidth;
    float h = screenHeight;

    Vector2 lines[] = {
        Vector2{1.0, 0.0},
        Vector2{-0.5, 0.5},
        Vector2{-0.5, -0.5},
    };

    static float scale = 20.0f;

    for(auto &l : lines)
    {
        l = Vector2Rotate(l, g_vars.player.rotation);
        l *= scale; //
        l += g_vars.player.pos;
        l += Vector2{w, h}  * 0.5;
    }

    // Triangle shapes and lines
    DrawTriangleLines(lines[0], lines[1], lines[2], VIOLET);
}


void drawBullets(int32_t screenWidth, int32_t screenHeight)
{
    float w = screenWidth;
    float h = screenHeight;

    for(Bullet& b : bullets)
    {
        DrawRectangle(int32_t(b.pos.x + w * 0.5), int32_t(b.pos.y + h * 0.5), 2, 2, RAYWHITE);
    }
}


void drawRocks(int32_t screenWidth, int32_t screenHeight)
{
    float w = screenWidth;
    float h = screenHeight;

    for(Rock& r : rocks)
    {
        DrawRectangle(int32_t(r.pos.x + w * 0.5), int32_t(r.pos.y + h * 0.5), 20, 20, RAYWHITE);
    }
}


