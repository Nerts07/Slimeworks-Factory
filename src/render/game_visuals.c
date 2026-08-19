#include "game_visuals.h"

typedef struct GameVisualDefinition {
    const char *path;
    float uniformScale;
    float verticalOffset;
} GameVisualDefinition;

static const GameVisualDefinition DEFINITIONS[GAME_VISUAL_COUNT] = {
#define GAME_VISUAL(id, path, scale, yOffset) [GAME_VISUAL_##id] = { path, scale, yOffset },
#include "visual_assets.def"
#undef GAME_VISUAL
};

void GameVisualsInit(GameVisuals *visuals)
{
    *visuals = (GameVisuals){ 0 };
    for (int i = 0; i < GAME_VISUAL_COUNT; ++i) {
        GameVisualAsset *asset = &visuals->assets[i];
        asset->uniformScale = DEFINITIONS[i].uniformScale;
        asset->verticalOffset = DEFINITIONS[i].verticalOffset;
        if (!FileExists(DEFINITIONS[i].path)) continue;
        asset->model = LoadModel(DEFINITIONS[i].path);
        asset->loaded = IsModelValid(asset->model);
        if (asset->loaded) TraceLog(LOG_INFO, "VISUAL: loaded %s", DEFINITIONS[i].path);
        else TraceLog(LOG_WARNING, "VISUAL: failed to load %s; using primitive fallback", DEFINITIONS[i].path);
    }
}

void GameVisualsShutdown(GameVisuals *visuals)
{
    for (int i = 0; i < GAME_VISUAL_COUNT; ++i) {
        if (visuals->assets[i].loaded) UnloadModel(visuals->assets[i].model);
    }
    *visuals = (GameVisuals){ 0 };
}

bool GameVisualsDraw(const GameVisuals *visuals, GameVisualKind kind, Vector3 position,
                     float rotationYDegrees, Vector3 scale, Color tint)
{
    if (!visuals || kind < 0 || kind >= GAME_VISUAL_COUNT || !visuals->assets[kind].loaded) return false;
    const GameVisualAsset *asset = &visuals->assets[kind];
    float baseScale = asset->uniformScale;
    Vector3 finalScale = { scale.x * baseScale, scale.y * baseScale, scale.z * baseScale };
    position.y += asset->verticalOffset * finalScale.y;
    DrawModelEx(asset->model, position, (Vector3){ 0.0f, 1.0f, 0.0f }, rotationYDegrees, finalScale, tint);
    return true;
}
