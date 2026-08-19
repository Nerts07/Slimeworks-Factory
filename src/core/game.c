#include "game.h"
#include "../ui/ui.h"
#include "../ui/world_selector.h"
#include <raylib.h>
#include <math.h>
#include <stdio.h>

static void GameDraw(const Game *game)
{
    Vector3 preview = GameplayBuildPreview(&game->gameplay);
    bool previewValid = GameplayCanPreviewBuilding(&game->gameplay, preview);
    BeginDrawing();
    ClearBackground((Color){ 185, 214, 225, 255 });
    BeginMode3D(game->playerSystem.camera);
    LevelDraw(&game->level, &game->visuals);
    GameStateDrawWorld(&game->state, &game->visuals, game->gameplay.buildMode, game->gameplay.selectedBuild, preview, previewValid);
    EndMode3D();
    const Building *nearby = BuildingsFindNearest(&game->state.buildings, game->playerSystem.player.position, 2.4f);
    UiDrawHud(&game->playerSystem.player, &game->state, game->playerSystem.mouseCaptured,
              game->gameplay.buildMode, game->gameplay.selectedBuild, GameplayTargetedNode(&game->gameplay), nearby,
              game->gameplay.harvestProgress, game->gameplay.notification, game->gameplay.notificationSeconds,
              game->gameplay.helpOpen);
    DeveloperConsoleDraw(&game->console);
    EndDrawing();
}

static void GameStartSession(Game *game, const WorldChoice *choice)
{
    if (choice->type == WORLD_CHOICE_GENERATED) {
        LevelGenerate(&game->level, choice->seed);
    } else if (!LevelLoadFromFile(&game->level, choice->levelPath)) {
        TraceLog(LOG_WARNING, "Could not load authored level %s; generating a fallback", choice->levelPath);
        LevelGenerate(&game->level, 24681357u);
    }
    char title[128];
    snprintf(title, sizeof(title), "Slimeworks Factory — %s", game->level.title);
    SetWindowTitle(title);
    InputGateInit(&game->inputGate);
    GameStateInit(&game->state, &game->level);
    ResourceNodesAlignToLevel(&game->state.nodes, &game->level);
    PlayerSystemInit(&game->playerSystem, &game->level, &game->inputGate, LevelGetSpawnPoint(&game->level));
    GameplaySystemInit(&game->gameplay, &game->state, &game->level, &game->playerSystem, &game->inputGate);
    DeveloperConsoleInit(&game->console, &game->inputGate, &game->state, &game->level, &game->playerSystem, &game->gameplay);
    UpdateLoopInit(&game->updateLoop);
    UpdateLoopRegister(&game->updateLoop, DeveloperConsoleSystemUpdate, &game->console);
    UpdateLoopRegister(&game->updateLoop, PlayerSystemUpdate, &game->playerSystem);
    UpdateLoopRegister(&game->updateLoop, GameplaySystemUpdate, &game->gameplay);
}

void GameInit(Game *game)
{
    *game = (Game){ 0 };
    InitWindow(1280, 720, "Slimeworks Factory");
    SetTargetFPS(60);
    GameVisualsInit(&game->visuals);
    LevelCatalogRefresh(&game->levelCatalog);
}

void GameRun(Game *game)
{
    WorldSelector selector;
    WorldSelectorInit(&selector, &game->levelCatalog);
    while (!WindowShouldClose() && !selector.confirmed && !selector.cancelled) {
        WorldSelectorUpdate(&selector);
        WorldSelectorDraw(&selector);
    }
    if (WindowShouldClose() || selector.cancelled) return;
    GameStartSession(game, &selector.choice);
    while (!WindowShouldClose()) {
        float deltaTime = fminf(GetFrameTime(), 0.05f);
        UpdateLoopRun(&game->updateLoop, deltaTime);
        GameDraw(game);
    }
}

void GameShutdown(Game *game)
{
    LevelShutdown(&game->level);
    GameVisualsShutdown(&game->visuals);
    CloseWindow();
}
