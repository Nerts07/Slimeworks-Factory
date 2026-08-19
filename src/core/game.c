#include "game.h"
#include "../ui/ui.h"
#include <raylib.h>
#include <math.h>

static void GameDraw(const Game *game)
{
    Vector3 preview = GameplayBuildPreview(&game->gameplay);
    bool previewValid = GameplayCanPreviewBuilding(&game->gameplay, preview);
    BeginDrawing();
    ClearBackground((Color){ 185, 214, 225, 255 });
    BeginMode3D(game->playerSystem.camera);
    LevelDraw(&game->level);
    GameStateDrawWorld(&game->state, game->gameplay.buildMode, game->gameplay.selectedBuild, preview, previewValid);
    EndMode3D();
    const Building *nearby = BuildingsFindNearest(&game->state.buildings, game->playerSystem.player.position, 2.4f);
    UiDrawHud(&game->playerSystem.player, &game->state, game->playerSystem.mouseCaptured,
              game->gameplay.buildMode, game->gameplay.selectedBuild, GameplayTargetedNode(&game->gameplay), nearby,
              game->gameplay.harvestProgress, game->gameplay.notification, game->gameplay.notificationSeconds,
              game->gameplay.helpOpen);
    DeveloperConsoleDraw(&game->console);
    EndDrawing();
}

void GameInit(Game *game)
{
    InitWindow(1280/1.2, 720/1.2, "Slimeworks Factory");
    SetTargetFPS(60);
    InputGateInit(&game->inputGate);
    LevelInit(&game->level);
    GameStateInit(&game->state);
    ResourceNodesAlignToLevel(&game->state.nodes, &game->level);
    PlayerSystemInit(&game->playerSystem, &game->level, &game->inputGate, LevelGetSpawnPoint(&game->level));
    GameplaySystemInit(&game->gameplay, &game->state, &game->level, &game->playerSystem, &game->inputGate);
    DeveloperConsoleInit(&game->console, &game->inputGate, &game->state, &game->level, &game->playerSystem, &game->gameplay);
    UpdateLoopInit(&game->updateLoop);
    UpdateLoopRegister(&game->updateLoop, DeveloperConsoleSystemUpdate, &game->console);
    UpdateLoopRegister(&game->updateLoop, PlayerSystemUpdate, &game->playerSystem);
    UpdateLoopRegister(&game->updateLoop, GameplaySystemUpdate, &game->gameplay);
}

void GameRun(Game *game)
{
    while (!WindowShouldClose()) {
        float deltaTime = fminf(GetFrameTime(), 0.05f);
        UpdateLoopRun(&game->updateLoop, deltaTime);
        GameDraw(game);
    }
}

void GameShutdown(Game *game)
{
    LevelShutdown(&game->level);
    CloseWindow();
}
