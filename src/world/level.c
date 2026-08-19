#include "level.h"
#include <math.h>
#include <raymath.h>
#include <string.h>

static void AddPlatform(Level *level, Vector3 center, Vector2 size, float top, Color color)
{
    if (level->platformCount < LEVEL_MAX_PLATFORMS)
        level->platforms[level->platformCount++] = (LevelPlatform){ center, size, top, color };
}

static void AddRamp(Level *level, Vector3 start, Vector3 end, float width, Color color)
{
    if (level->rampCount < LEVEL_MAX_RAMPS)
        level->ramps[level->rampCount++] = (LevelRamp){ start, end, width, color };
}

static int AddCollider(Level *level, Vector3 position, Vector3 size, Color color)
{
    if (level->boxCount >= LEVEL_MAX_BOXES) return -1;
    int index = level->boxCount;
    level->boxes[level->boxCount++] = (LevelBox){ position, size, color, true };
    return index;
}

void LevelInit(Level *level)
{
    *level = (Level){ .gateColliderIndex = -1 };

    /* A large lower floor keeps exploration open while raised districts remain readable. */
    AddPlatform(level, (Vector3){ 0, 0, 0 }, (Vector2){ 124, 124 }, -1.5f, (Color){ 42, 91, 103, 255 });
    AddPlatform(level, (Vector3){ 0, 0, 0 }, (Vector2){ 30, 30 }, 0.0f, (Color){ 61, 161, 111, 255 });
    AddPlatform(level, (Vector3){ -40, 0, 5 }, (Vector2){ 32, 34 }, 1.5f, (Color){ 103, 193, 100, 255 });
    AddPlatform(level, (Vector3){ 0, 0, 40 }, (Vector2){ 34, 30 }, 3.0f, (Color){ 90, 159, 217, 255 });
    AddPlatform(level, (Vector3){ 40, 0, 0 }, (Vector2){ 32, 30 }, 2.0f, (Color){ 166, 199, 70, 255 });
    AddPlatform(level, (Vector3){ 40, 0, -40 }, (Vector2){ 32, 28 }, 5.0f, (Color){ 147, 99, 205, 255 });
    AddPlatform(level, (Vector3){ 40, 0, 40 }, (Vector2){ 30, 28 }, 4.0f, (Color){ 229, 142, 73, 255 });

    AddRamp(level, (Vector3){ -14, 0.0f, 4 }, (Vector3){ -24, 1.5f, 4 }, 8.0f, (Color){ 93, 183, 117, 255 });
    AddRamp(level, (Vector3){ 0, 0.0f, 14 }, (Vector3){ 0, 3.0f, 25 }, 8.0f, (Color){ 83, 150, 196, 255 });
    AddRamp(level, (Vector3){ 14, 0.0f, -3 }, (Vector3){ 25, 2.0f, -3 }, 9.0f, (Color){ 147, 191, 72, 255 });
    AddRamp(level, (Vector3){ 40, 2.0f, -14 }, (Vector3){ 40, 5.0f, -26 }, 8.0f, (Color){ 134, 90, 191, 255 });
    AddRamp(level, (Vector3){ 40, 2.0f, 14 }, (Vector3){ 40, 4.0f, 26 }, 8.0f, (Color){ 213, 126, 65, 255 });

    /* Purposeful landmarks also act as collision volumes. */
    AddCollider(level, (Vector3){ 0, 1.5f, 0 }, (Vector3){ 4.5f, 3.0f, 4.5f }, (Color){ 52, 205, 128, 255 });
    AddCollider(level, (Vector3){ -43, 3.1f, 7 }, (Vector3){ 5.0f, 3.2f, 5.0f }, (Color){ 236, 91, 161, 255 });
    AddCollider(level, (Vector3){ 0, 5.0f, 43 }, (Vector3){ 5.0f, 4.0f, 5.0f }, (Color){ 83, 166, 232, 255 });
    level->gateColliderIndex = AddCollider(level, (Vector3){ 20, 1.5f, -3 }, (Vector3){ 0.25f, 3.0f, 8.5f }, (Color){ 236, 76, 151, 255 });
}

void LevelShutdown(Level *level)
{
    (void)level;
}

static bool PlatformContains(const LevelPlatform *platform, float x, float z)
{
    return fabsf(x - platform->center.x) <= platform->size.x * 0.5f &&
           fabsf(z - platform->center.z) <= platform->size.y * 0.5f;
}

static bool RampSample(const LevelRamp *ramp, float x, float z, float *outHeight, float *outSlope)
{
    Vector2 start = { ramp->start.x, ramp->start.z };
    Vector2 end = { ramp->end.x, ramp->end.z };
    Vector2 delta = Vector2Subtract(end, start);
    float length = Vector2Length(delta);
    if (length < 0.001f) return false;
    Vector2 direction = Vector2Scale(delta, 1.0f / length);
    Vector2 relative = { x - start.x, z - start.y };
    float along = Vector2DotProduct(relative, direction);
    float side = fabsf(relative.x * -direction.y + relative.y * direction.x);
    if (along < 0.0f || along > length || side > ramp->width * 0.5f) return false;
    float t = along / length;
    *outHeight = Lerp(ramp->start.y, ramp->end.y, t);
    *outSlope = atan2f(fabsf(ramp->end.y - ramp->start.y), length) * RAD2DEG;
    return true;
}

GroundSample LevelSampleGround(const Level *level, float x, float z)
{
    GroundSample sample = { -1.5f, 0.0f, GROUND_FLAT };
    for (int i = 0; i < level->platformCount; ++i) {
        const LevelPlatform *platform = &level->platforms[i];
        if (PlatformContains(platform, x, z) && platform->top > sample.height)
            sample = (GroundSample){ platform->top, 0.0f, GROUND_FLAT };
    }
    for (int i = 0; i < level->rampCount; ++i) {
        float height, slope;
        if (RampSample(&level->ramps[i], x, z, &height, &slope) && height >= sample.height - 0.01f)
            sample = (GroundSample){ height, slope, GROUND_RAMP };
    }
    return sample;
}

float LevelGroundHeight(const Level *level, float x, float z)
{
    return LevelSampleGround(level, x, z).height;
}

Vector3 LevelGetSpawnPoint(const Level *level)
{
    Vector3 spawn = { 0.0f, 0.0f, 4.5f };
    spawn.y = LevelGroundHeight(level, spawn.x, spawn.z);
    return spawn;
}

bool LevelGetLandmark(const Level *level, const char *name, Vector3 *outPosition)
{
    static const struct { const char *name; float x; float z; } landmarks[] = {
        { "hub", 0.0f, 4.5f }, { "ranch", -40.0f, 5.0f }, { "lab", 0.0f, 40.0f },
        { "acid", 40.0f, 0.0f }, { "crystal", 40.0f, -40.0f }, { "range", 40.0f, 40.0f }
    };
    for (int i = 0; i < (int)(sizeof(landmarks) / sizeof(landmarks[0])); ++i) if (strcmp(name, landmarks[i].name) == 0) {
        *outPosition = (Vector3){ landmarks[i].x, LevelGroundHeight(level, landmarks[i].x, landmarks[i].z), landmarks[i].z };
        return true;
    }
    return false;
}

bool LevelCollides(const Level *level, float x, float z, float radius)
{
    if (fabsf(x) > 60.5f - radius || fabsf(z) > 60.5f - radius) return true;
    for (int i = 0; i < level->boxCount; ++i) {
        if (i == level->gateColliderIndex && level->zoneTwoUnlocked) continue;
        const LevelBox *box = &level->boxes[i];
        float cx = Clamp(x, box->position.x - box->size.x * 0.5f, box->position.x + box->size.x * 0.5f);
        float cz = Clamp(z, box->position.z - box->size.z * 0.5f, box->position.z + box->size.z * 0.5f);
        float dx = x - cx, dz = z - cz;
        if (dx * dx + dz * dz < radius * radius) return true;
    }
    return false;
}

static void DrawPlatform(const LevelPlatform *platform)
{
    float bottom = -2.2f;
    float thickness = platform->top - bottom;
    Vector3 center = { platform->center.x, bottom + thickness * 0.5f, platform->center.z };
    Vector3 size = { platform->size.x, thickness, platform->size.y };
    DrawCubeV(center, size, platform->color);
    DrawCubeWiresV(center, size, Fade(WHITE, 0.12f));
}

static void DrawRamp(const LevelRamp *ramp)
{
    Vector2 delta = { ramp->end.x - ramp->start.x, ramp->end.z - ramp->start.z };
    float length = Vector2Length(delta);
    Vector2 direction = Vector2Scale(delta, 1.0f / length);
    Vector2 side = { -direction.y * ramp->width * 0.5f, direction.x * ramp->width * 0.5f };
    float bottom = fminf(ramp->start.y, ramp->end.y) - 0.28f;
    Vector3 a = { ramp->start.x + side.x, ramp->start.y, ramp->start.z + side.y };
    Vector3 b = { ramp->start.x - side.x, ramp->start.y, ramp->start.z - side.y };
    Vector3 c = { ramp->end.x - side.x, ramp->end.y, ramp->end.z - side.y };
    Vector3 d = { ramp->end.x + side.x, ramp->end.y, ramp->end.z + side.y };
    Vector3 ab = { a.x, bottom, a.z }, bb = { b.x, bottom, b.z }, cb = { c.x, bottom, c.z }, db = { d.x, bottom, d.z };
    DrawTriangle3D(a, d, c, ramp->color); DrawTriangle3D(a, c, b, ramp->color);
    DrawTriangle3D(ab, bb, cb, Fade(ramp->color, 0.8f)); DrawTriangle3D(ab, cb, db, Fade(ramp->color, 0.8f));
    DrawTriangle3D(a, ab, bb, Fade(ramp->color, 0.9f)); DrawTriangle3D(a, bb, b, Fade(ramp->color, 0.9f));
    DrawTriangle3D(d, c, cb, Fade(ramp->color, 0.9f)); DrawTriangle3D(d, cb, db, Fade(ramp->color, 0.9f));
    DrawLine3D(a, d, WHITE); DrawLine3D(b, c, WHITE);
}

static void DrawSlime(Vector3 position, float scale, Color color)
{
    DrawSphere((Vector3){ position.x, position.y + scale * 0.65f, position.z }, scale, color);
    DrawSphere((Vector3){ position.x - scale * 0.36f, position.y + scale * 0.82f, position.z - scale * 0.82f }, scale * 0.2f, WHITE);
    DrawSphere((Vector3){ position.x + scale * 0.36f, position.y + scale * 0.82f, position.z - scale * 0.82f }, scale * 0.2f, WHITE);
    DrawSphere((Vector3){ position.x - scale * 0.36f, position.y + scale * 0.82f, position.z - scale }, scale * 0.09f, BLACK);
    DrawSphere((Vector3){ position.x + scale * 0.36f, position.y + scale * 0.82f, position.z - scale }, scale * 0.09f, BLACK);
}

static void DrawSilo(Vector3 p, Color color)
{
    DrawCylinder((Vector3){ p.x, p.y + 1.5f, p.z }, 1.15f, 1.15f, 3.0f, 18, color);
    DrawSphere((Vector3){ p.x, p.y + 3.0f, p.z }, 1.15f, Fade(WHITE, 0.75f));
    DrawCylinderWires((Vector3){ p.x, p.y + 1.5f, p.z }, 1.2f, 1.2f, 3.0f, 18, DARKGRAY);
}

static void DrawDistrictDecorations(const Level *level)
{
    /* Central factory. */
    DrawCylinder((Vector3){ 0, 1.5f, 0 }, 2.2f, 1.6f, 3.0f, 24, (Color){ 52, 205, 128, 255 });
    DrawSphere((Vector3){ 0, 3.25f, 0 }, 1.25f, (Color){ 110, 255, 176, 255 });
    DrawSilo((Vector3){ -9, 0, -9 }, PINK); DrawSilo((Vector3){ 9, 0, 9 }, SKYBLUE);

    /* West ranch: giant mascot and bouncing-pit markers. */
    DrawSlime((Vector3){ -43, 1.5f, 7 }, 2.7f, (Color){ 238, 88, 163, 255 });
    for (int i = 0; i < 6; ++i) {
        float angle = i * PI / 3.0f;
        DrawCylinder((Vector3){ -40 + cosf(angle) * 10, 1.75f, 5 + sinf(angle) * 10 }, 0.15f, 0.15f, 0.5f, 8, GOLD);
    }

    /* North lab/market skyline. */
    DrawSilo((Vector3){ -9, 3, 43 }, BLUE); DrawSilo((Vector3){ 9, 3, 43 }, VIOLET);
    DrawCube((Vector3){ 0, 5.0f, 43 }, 5.0f, 4.0f, 5.0f, (Color){ 83, 166, 232, 255 });
    DrawCubeWires((Vector3){ 0, 5.0f, 43 }, 5.1f, 4.1f, 5.1f, WHITE);

    /* East acid works. */
    for (int i = -1; i <= 1; ++i) {
        DrawCylinder((Vector3){ 40 + i * 6.0f, 2.35f, 4.5f }, 2.0f, 2.0f, 0.7f, 24, DARKGREEN);
        DrawCylinder((Vector3){ 40 + i * 6.0f, 2.72f, 4.5f }, 1.75f, 1.75f, 0.08f, 24, LIME);
    }

    /* Crystal quarry and orange test range. */
    static const Vector2 crystals[] = { {31,-43}, {36,-34}, {44,-45}, {50,-37}, {43,-52} };
    for (int i = 0; i < 5; ++i) {
        DrawCylinder((Vector3){ crystals[i].x, 6.0f, crystals[i].y }, 0.18f, 0.75f, 2.0f + (i%2), 5, VIOLET);
        DrawSphere((Vector3){ crystals[i].x, 7.2f + (i%2), crystals[i].y }, 0.28f, GOLD);
    }
    DrawSlime((Vector3){ 40, 4.0f, 42 }, 2.1f, ORANGE);

    /* Locked neon gate on the east ramp. */
    DrawCylinder((Vector3){ 20, 1.5f, -7.1f }, 0.18f, 0.18f, 3.0f, 10, PINK);
    DrawCylinder((Vector3){ 20, 1.5f, 1.1f }, 0.18f, 0.18f, 3.0f, 10, PINK);
    DrawCube((Vector3){ 20, 3.0f, -3 }, 0.32f, 0.3f, 8.5f, VIOLET);
    if (!level->zoneTwoUnlocked) DrawCube((Vector3){ 20, 1.5f, -3 }, 0.12f, 2.8f, 7.9f, Fade(RED, 0.48f));

    /* Tall color-coded beacons provide orientation across the large map. */
    DrawCylinder((Vector3){ -52, 5.0f, -8 }, 0.18f, 0.18f, 7.0f, 8, PINK);
    DrawSphere((Vector3){ -52, 8.7f, -8 }, 0.65f, GOLD);
    DrawCylinder((Vector3){ 54, 6.0f, 10 }, 0.18f, 0.18f, 8.0f, 8, LIME);
    DrawSphere((Vector3){ 54, 10.2f, 10 }, 0.65f, YELLOW);
    DrawCylinder((Vector3){ 28, 9.0f, -52 }, 0.18f, 0.18f, 8.0f, 8, VIOLET);
    DrawSphere((Vector3){ 28, 13.2f, -52 }, 0.65f, WHITE);
}

void LevelDraw(const Level *level)
{
    for (int i = 0; i < level->platformCount; ++i) DrawPlatform(&level->platforms[i]);
    for (int i = 0; i < level->rampCount; ++i) DrawRamp(&level->ramps[i]);
    DrawDistrictDecorations(level);
}
