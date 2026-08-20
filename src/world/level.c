#include "level.h"
#include <math.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>

static void CopyLabel(char *destination, size_t destinationSize, const char *prefix, int index)
{
    snprintf(destination, destinationSize, "%s_%d", prefix, index);
}

static void SetFaceColors(Color faceColors[6], Color color)
{
    for (int i = 0; i < 6; ++i) faceColors[i] = color;
}

static void AddPlatform(Level *level, Vector3 center, Vector2 size, float top, Color color)
{
    if (level->platformCount >= LEVEL_MAX_PLATFORMS) return;
    LevelPlatform *platform = &level->platforms[level->platformCount];
    *platform = (LevelPlatform){ .center = center, .size = size, .top = top, .color = color };
    CopyLabel(platform->name, sizeof(platform->name), "platform", level->platformCount);
    strcpy(platform->tag, "ground");
    level->platformCount++;
}

static void AddRamp(Level *level, Vector3 start, Vector3 end, float width, Color color)
{
    if (level->rampCount >= LEVEL_MAX_RAMPS) return;
    LevelRamp *ramp = &level->ramps[level->rampCount];
    *ramp = (LevelRamp){ .start = start, .end = end, .width = width, .color = color };
    CopyLabel(ramp->name, sizeof(ramp->name), "ramp", level->rampCount);
    strcpy(ramp->tag, "ground_ramp");
    level->rampCount++;
}

static int AddCollider(Level *level, Vector3 position, Vector3 size, Color color)
{
    if (level->boxCount >= LEVEL_MAX_BOXES) return -1;
    int index = level->boxCount;
    LevelBox *box = &level->boxes[level->boxCount++];
    *box = (LevelBox){ .position = position, .size = size, .subdivisions = 1, .collidable = true, .visible = false };
    CopyLabel(box->name, sizeof(box->name), "collider", index);
    strcpy(box->tag, "collision");
    SetFaceColors(box->faceColors, color);
    return index;
}

static void AddMarker(Level *level, const char *name, const char *tag, Vector3 position)
{
    if (level->markerCount >= LEVEL_MAX_MARKERS) return;
    LevelMarker *marker = &level->markers[level->markerCount++];
    *marker = (LevelMarker){ .position = position };
    snprintf(marker->name, sizeof(marker->name), "%s", name);
    snprintf(marker->tag, sizeof(marker->tag), "%s", tag);
}

static float GeneratedRange(unsigned int *state, float minimum, float maximum)
{
    *state = *state * 1664525u + 1013904223u;
    float unit = (float)((*state >> 8) & 0x00ffffffu) / 16777215.0f;
    return Lerp(minimum, maximum, unit);
}

static Vector3 RotateGeneratedPoint(Vector3 point, int turns)
{
    switch (turns & 3) {
        case 1: return (Vector3){ -point.z, point.y, point.x };
        case 2: return (Vector3){ -point.x, point.y, -point.z };
        case 3: return (Vector3){ point.z, point.y, -point.x };
        default: return point;
    }
}

void LevelGenerate(Level *level, unsigned int seed)
{
    if (seed == 0) seed = 1;
    *level = (Level){ .gateColliderIndex = -1, .generated = true, .seed = seed };
    snprintf(level->title, sizeof(level->title), "Generated Expedition %06X", seed & 0x00ffffffu);

    static const Color themes[][7] = {
        {{ 42, 91, 103, 255 }, { 61, 161, 111, 255 }, { 103, 193, 100, 255 }, { 90, 159, 217, 255 }, { 166, 199, 70, 255 }, { 147, 99, 205, 255 }, { 229, 142, 73, 255 }},
        {{ 46, 67, 122, 255 }, { 86, 142, 212, 255 }, { 80, 196, 189, 255 }, { 122, 111, 222, 255 }, { 232, 180, 70, 255 }, { 224, 96, 177, 255 }, { 92, 198, 128, 255 }},
        {{ 86, 82, 112, 255 }, { 93, 187, 137, 255 }, { 202, 140, 78, 255 }, { 77, 181, 204, 255 }, { 214, 203, 90, 255 }, { 181, 106, 218, 255 }, { 236, 116, 114, 255 }}
    };
    const Color *colors = themes[seed % 3u];
    unsigned int randomState = seed;
    int turns = (int)(seed % 4u);
    float westTop = GeneratedRange(&randomState, 1.15f, 1.85f);
    float northTop = GeneratedRange(&randomState, 2.55f, 3.35f);
    float eastTop = GeneratedRange(&randomState, 1.65f, 2.35f);
    float crystalTop = GeneratedRange(&randomState, 4.65f, 5.35f);
    float rangeTop = GeneratedRange(&randomState, 3.65f, 4.35f);

    AddPlatform(level, (Vector3){ 0, 0, 0 }, (Vector2){ 124, 124 }, -1.5f, colors[0]);
    AddPlatform(level, (Vector3){ 0, 0, 0 }, (Vector2){ 30, 30 }, 0.0f, colors[1]);
    AddPlatform(level, RotateGeneratedPoint((Vector3){ -40, 0, 5 }, turns), (Vector2){ 32, 34 }, westTop, colors[2]);
    AddPlatform(level, RotateGeneratedPoint((Vector3){ 0, 0, 40 }, turns), (Vector2){ 34, 30 }, northTop, colors[3]);
    AddPlatform(level, RotateGeneratedPoint((Vector3){ 40, 0, 0 }, turns), (Vector2){ 32, 30 }, eastTop, colors[4]);
    AddPlatform(level, RotateGeneratedPoint((Vector3){ 40, 0, -40 }, turns), (Vector2){ 32, 28 }, crystalTop, colors[5]);
    AddPlatform(level, RotateGeneratedPoint((Vector3){ 40, 0, 40 }, turns), (Vector2){ 30, 28 }, rangeTop, colors[6]);

    AddRamp(level, RotateGeneratedPoint((Vector3){ -14, 0.0f, 4 }, turns), RotateGeneratedPoint((Vector3){ -24, westTop, 4 }, turns), 8.0f, colors[2]);
    AddRamp(level, RotateGeneratedPoint((Vector3){ 0, 0.0f, 14 }, turns), RotateGeneratedPoint((Vector3){ 0, northTop, 25 }, turns), 8.0f, colors[3]);
    AddRamp(level, RotateGeneratedPoint((Vector3){ 14, 0.0f, -3 }, turns), RotateGeneratedPoint((Vector3){ 25, eastTop, -3 }, turns), 9.0f, colors[4]);
    AddRamp(level, RotateGeneratedPoint((Vector3){ 40, eastTop, -14 }, turns), RotateGeneratedPoint((Vector3){ 40, crystalTop, -26 }, turns), 8.0f, colors[5]);
    AddRamp(level, RotateGeneratedPoint((Vector3){ 40, eastTop, 14 }, turns), RotateGeneratedPoint((Vector3){ 40, rangeTop, 26 }, turns), 8.0f, colors[6]);

    AddCollider(level, RotateGeneratedPoint((Vector3){ 0, 1.5f, 0 }, turns), (Vector3){ 4.5f, 3.0f, 4.5f }, colors[1]);
    AddCollider(level, RotateGeneratedPoint((Vector3){ -43, westTop + 1.6f, 7 }, turns), (Vector3){ 5.0f, 3.2f, 5.0f }, colors[2]);
    AddCollider(level, RotateGeneratedPoint((Vector3){ 0, northTop + 2.0f, 43 }, turns), (Vector3){ 5.0f, 4.0f, 5.0f }, colors[3]);
    Vector3 gateSize = (turns & 1) ? (Vector3){ 8.5f, 3.0f, 0.25f } : (Vector3){ 0.25f, 3.0f, 8.5f };
    level->gateColliderIndex = AddCollider(level, RotateGeneratedPoint((Vector3){ 20, 1.5f, -3 }, turns), gateSize, colors[5]);

    AddMarker(level, "player_start", "player_spawn", RotateGeneratedPoint((Vector3){ 0, 0, 6 }, turns));
    AddMarker(level, "green_a", "ore_green", RotateGeneratedPoint((Vector3){ -8, 0, 8 }, turns));
    AddMarker(level, "green_b", "ore_green", RotateGeneratedPoint((Vector3){ 8, 0, 8 }, turns));
    AddMarker(level, "green_c", "ore_green", RotateGeneratedPoint((Vector3){ -8, 0, -8 }, turns));
    AddMarker(level, "green_d", "ore_green", RotateGeneratedPoint((Vector3){ 8, 0, -8 }, turns));
    AddMarker(level, "acid_a", "ore_acid", RotateGeneratedPoint((Vector3){ 34, 0, 5 }, turns));
    AddMarker(level, "acid_b", "ore_acid", RotateGeneratedPoint((Vector3){ 47, 0, -6 }, turns));
    AddMarker(level, "crystal_a", "ore_crystal", RotateGeneratedPoint((Vector3){ 45, 0, -40 }, turns));
    AddMarker(level, "zone_two_gate", "zone_two_gate", RotateGeneratedPoint((Vector3){ 20, 0, -3 }, turns));
    AddMarker(level, "rare_slime", "rare_slime_spawn", RotateGeneratedPoint((Vector3){ 38, 0, -43 }, turns));
}

void LevelInit(Level *level)
{
    LevelGenerate(level, 24681357u);
}

void LevelShutdown(Level *level)
{
    for (int i = 0; i < level->modelCount; ++i) {
        if (level->models[i].modelLoaded) UnloadModel(level->models[i].model);
    }
    level->modelCount = 0;
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
    Vector3 marker;
    if (LevelFindMarker(level, "player_spawn", &marker)) return marker;
    Vector3 spawn = { 0.0f, 0.0f, 4.5f };
    spawn.y = LevelGroundHeight(level, spawn.x, spawn.z);
    return spawn;
}

bool LevelGetLandmark(const Level *level, const char *name, Vector3 *outPosition)
{
    if (LevelFindMarker(level, name, outPosition)) return true;
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

bool LevelFindMarker(const Level *level, const char *nameOrTag, Vector3 *outPosition)
{
    for (int i = 0; i < level->markerCount; ++i) {
        const LevelMarker *marker = &level->markers[i];
        if (strcmp(marker->name, nameOrTag) == 0 || strcmp(marker->tag, nameOrTag) == 0) {
            if (outPosition) *outPosition = marker->position;
            return true;
        }
    }
    return false;
}

int LevelFindMarkersWithTag(const Level *level, const char *tag, Vector3 *outPositions, int maxPositions)
{
    int found = 0;
    for (int i = 0; i < level->markerCount; ++i) {
        if (strcmp(level->markers[i].tag, tag) != 0) continue;
        if (outPositions && found < maxPositions) outPositions[found] = level->markers[i].position;
        found++;
    }
    return found;
}

const LevelModel *LevelFindModel(const Level *level, const char *nameOrTag)
{
    for (int i = 0; i < level->modelCount; ++i) {
        const LevelModel *model = &level->models[i];
        if (strcmp(model->name, nameOrTag) == 0 || strcmp(model->tag, nameOrTag) == 0) return model;
    }
    return NULL;
}

int LevelFindModelsWithTag(const Level *level, const char *tag, const LevelModel **outModels, int maxModels)
{
    int found = 0;
    for (int i = 0; i < level->modelCount; ++i) {
        if (strcmp(level->models[i].tag, tag) != 0) continue;
        if (outModels && found < maxModels) outModels[found] = &level->models[i];
        found++;
    }
    return found;
}

bool LevelCollides(const Level *level, float x, float z, float radius)
{
    if (fabsf(x) > 60.5f - radius || fabsf(z) > 60.5f - radius) return true;
    for (int i = 0; i < level->boxCount; ++i) {
        if (i == level->gateColliderIndex && level->zoneTwoUnlocked) continue;
        const LevelBox *box = &level->boxes[i];
        if (!box->collidable) continue;
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

static Vector3 TransformBoxPoint(const LevelBox *box, Vector3 local)
{
    Matrix rotation = MatrixRotateXYZ(Vector3Scale(box->rotation, DEG2RAD));
    return Vector3Add(Vector3Transform(local, rotation), box->position);
}

static Vector3 LerpVector3(Vector3 a, Vector3 b, float t)
{
    return (Vector3){ Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t) };
}

static void DrawSubdividedQuad(Vector3 a, Vector3 b, Vector3 c, Vector3 d, Color color, int subdivisions)
{
    int steps = Clamp(subdivisions, 1, 16);
    for (int y = 0; y < steps; ++y) {
        float v0 = (float)y / (float)steps, v1 = (float)(y + 1) / (float)steps;
        Vector3 left0 = LerpVector3(a, d, v0), right0 = LerpVector3(b, c, v0);
        Vector3 left1 = LerpVector3(a, d, v1), right1 = LerpVector3(b, c, v1);
        for (int x = 0; x < steps; ++x) {
            float u0 = (float)x / (float)steps, u1 = (float)(x + 1) / (float)steps;
            Vector3 p0 = LerpVector3(left0, right0, u0), p1 = LerpVector3(left0, right0, u1);
            Vector3 p2 = LerpVector3(left1, right1, u1), p3 = LerpVector3(left1, right1, u0);
            DrawTriangle3D(p0, p1, p2, color);
            DrawTriangle3D(p0, p2, p3, color);
            if (steps > 1) {
                Color wire = Fade(WHITE, 0.16f);
                DrawLine3D(p0, p1, wire); DrawLine3D(p1, p2, wire);
            }
        }
    }
}

static void DrawLevelBox(const LevelBox *box)
{
    float hx = box->size.x * 0.5f, hy = box->size.y * 0.5f, hz = box->size.z * 0.5f;
    float inset = Clamp(box->bevel, 0.0f, fminf(hx, fminf(hy, hz)) * 0.45f);
    if (inset > 0.001f) {
        Vector3 center = TransformBoxPoint(box, (Vector3){ 0 });
        DrawCubeV(center, box->size, Fade(BLACK, 0.45f));
    }
    Vector3 faces[6][4] = {
        {{-hx + inset, hy, -hz + inset}, {hx - inset, hy, -hz + inset}, {hx - inset, hy, hz - inset}, {-hx + inset, hy, hz - inset}},
        {{-hx + inset, -hy, hz - inset}, {hx - inset, -hy, hz - inset}, {hx - inset, -hy, -hz + inset}, {-hx + inset, -hy, -hz + inset}},
        {{-hx + inset, -hy + inset, -hz}, {hx - inset, -hy + inset, -hz}, {hx - inset, hy - inset, -hz}, {-hx + inset, hy - inset, -hz}},
        {{hx - inset, -hy + inset, hz}, {-hx + inset, -hy + inset, hz}, {-hx + inset, hy - inset, hz}, {hx - inset, hy - inset, hz}},
        {{hx, -hy + inset, -hz + inset}, {hx, -hy + inset, hz - inset}, {hx, hy - inset, hz - inset}, {hx, hy - inset, -hz + inset}},
        {{-hx, -hy + inset, hz - inset}, {-hx, -hy + inset, -hz + inset}, {-hx, hy - inset, -hz + inset}, {-hx, hy - inset, hz - inset}}
    };
    for (int i = 0; i < 6; ++i) {
        DrawSubdividedQuad(TransformBoxPoint(box, faces[i][0]), TransformBoxPoint(box, faces[i][1]),
                           TransformBoxPoint(box, faces[i][2]), TransformBoxPoint(box, faces[i][3]),
                           box->faceColors[i], box->subdivisions);
    }
}

static void DrawLevelModel(const LevelModel *levelModel)
{
    if (!levelModel->modelLoaded) return;
    Model model = levelModel->model;
    Matrix rotation = MatrixRotateXYZ(Vector3Scale(levelModel->rotation, DEG2RAD));
    model.transform = MatrixMultiply(MatrixScale(levelModel->scale.x, levelModel->scale.y, levelModel->scale.z), rotation);
    DrawModel(model, levelModel->position, 1.0f, WHITE);
}

static void DrawLevelMesh(const LevelMesh *mesh)
{
    for (int faceIndex = 0; faceIndex < mesh->faceCount; ++faceIndex) {
        const LevelMeshFace *face = &mesh->faces[faceIndex];
        if (face->vertexCount < 3 || face->vertexCount > LEVEL_MESH_FACE_MAX_VERTICES) continue;
        Vector3 a = Vector3Add(mesh->position, mesh->vertices[face->vertices[0]]);
        for (int vertex = 1; vertex + 1 < face->vertexCount; ++vertex) {
            Vector3 b = Vector3Add(mesh->position, mesh->vertices[face->vertices[vertex]]);
            Vector3 c = Vector3Add(mesh->position, mesh->vertices[face->vertices[vertex + 1]]);
            DrawTriangle3D(a, b, c, face->color);
        }
    }
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

static void DrawDistrictDecorations(const Level *level, const GameVisuals *visuals)
{
    if (!level->generated) return;
    for (int i = 1; i < level->platformCount; ++i) {
        const LevelPlatform *platform = &level->platforms[i];
        Vector3 p = { platform->center.x, platform->top, platform->center.z };
        Color accent = ColorBrightness(platform->color, 0.22f);
        if (i == 1) {
            if (!GameVisualsDraw(visuals, GAME_VISUAL_HUB, p, 0.0f, (Vector3){ 1, 1, 1 }, WHITE)) {
                DrawCylinder((Vector3){ p.x, p.y + 1.5f, p.z }, 2.2f, 1.6f, 3.0f, 24, accent);
                DrawSphere((Vector3){ p.x, p.y + 3.25f, p.z }, 1.25f, Fade(WHITE, 0.78f));
            }
        } else if ((i & 1) == 0) {
            if (!GameVisualsDraw(visuals, GAME_VISUAL_SLIME_MASCOT, p, 0.0f, (Vector3){ 1, 1, 1 }, WHITE))
                DrawSlime(p, 1.55f + (float)(i % 3) * 0.25f, accent);
        } else {
            if (!GameVisualsDraw(visuals, GAME_VISUAL_SILO, p, 0.0f, (Vector3){ 1, 1, 1 }, WHITE)) DrawSilo(p, accent);
        }
        if (!GameVisualsDraw(visuals, GAME_VISUAL_BEACON, p, 0.0f, (Vector3){ 1, 1, 1 }, WHITE)) {
            DrawCylinder((Vector3){ p.x, p.y + 5.0f, p.z }, 0.15f, 0.15f, 7.0f, 8, accent);
            DrawSphere((Vector3){ p.x, p.y + 8.7f, p.z }, 0.55f, GOLD);
        }
    }
    Vector3 gate;
    if (LevelFindMarker(level, "zone_two_gate", &gate)) {
        gate.y = LevelGroundHeight(level, gate.x, gate.z);
        float sideX = 0.0f, sideZ = 3.8f;
        if (level->gateColliderIndex >= 0 && level->boxes[level->gateColliderIndex].size.x > level->boxes[level->gateColliderIndex].size.z) {
            sideX = 3.8f;
            sideZ = 0.0f;
        }
        float gateRotation = sideX > 0.0f ? 90.0f : 0.0f;
        if (!GameVisualsDraw(visuals, GAME_VISUAL_ZONE_GATE, gate, gateRotation, (Vector3){ 1, 1, 1 }, WHITE)) {
            DrawCylinder((Vector3){ gate.x - sideX, gate.y + 1.5f, gate.z - sideZ }, 0.18f, 0.18f, 3.0f, 10, PINK);
            DrawCylinder((Vector3){ gate.x + sideX, gate.y + 1.5f, gate.z + sideZ }, 0.18f, 0.18f, 3.0f, 10, PINK);
        }
        if (!level->zoneTwoUnlocked) DrawSphere((Vector3){ gate.x, gate.y + 2.3f, gate.z }, 1.15f, Fade(RED, 0.48f));
    }
}

void LevelDraw(const Level *level, const GameVisuals *visuals)
{
    for (int i = 0; i < level->platformCount; ++i) DrawPlatform(&level->platforms[i]);
    for (int i = 0; i < level->rampCount; ++i) DrawRamp(&level->ramps[i]);
    for (int i = 0; i < level->boxCount; ++i) if (level->boxes[i].visible) DrawLevelBox(&level->boxes[i]);
    for (int i = 0; i < level->modelCount; ++i) DrawLevelModel(&level->models[i]);
    for (int i = 0; i < level->meshCount; ++i) DrawLevelMesh(&level->meshes[i]);
    DrawDistrictDecorations(level, visuals);
}
