#ifndef SLIMEWORKS_GAME_VISUALS_H
#define SLIMEWORKS_GAME_VISUALS_H

#include <raylib.h>
#include <stdbool.h>

typedef enum GameVisualKind {
#define GAME_VISUAL(id, path, scale, yOffset) GAME_VISUAL_##id,
#include "visual_assets.def"
#undef GAME_VISUAL
    GAME_VISUAL_COUNT
} GameVisualKind;

typedef struct GameVisualAsset {
    Model model;
    bool loaded;
    float uniformScale;
    float verticalOffset;
} GameVisualAsset;

typedef struct GameVisuals {
    GameVisualAsset assets[GAME_VISUAL_COUNT];
} GameVisuals;

void GameVisualsInit(GameVisuals *visuals);
void GameVisualsShutdown(GameVisuals *visuals);

/* Returns false when no model exists, allowing the caller to draw its current primitive fallback. */
bool GameVisualsDraw(const GameVisuals *visuals, GameVisualKind kind, Vector3 position,
                     float rotationYDegrees, Vector3 scale, Color tint);

#endif
