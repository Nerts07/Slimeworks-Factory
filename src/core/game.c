#include "game.h"
#include "../ui/ui.h"
#include "../ui/world_selector.h"
#include <raylib.h>
#include <math.h>
#include <stdio.h>

typedef struct DisplayResolution {
    int width;
    int height;
} DisplayResolution;

static const DisplayResolution DISPLAY_RESOLUTIONS[] = {
    { 1280, 720 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 }
};

static void GameApplyMonitorRefreshRate(void)
{
    int refreshRate = GetMonitorRefreshRate(GetCurrentMonitor());
    if (refreshRate < 30) refreshRate = 60;
    SetTargetFPS(refreshRate);
}

static void GameHandleDisplayHotkeys(void)
{
    static int windowedWidth = 1280;
    static int windowedHeight = 720;
    if (IsKeyPressed(KEY_F11)) {
        if (IsWindowFullscreen()) {
            ToggleFullscreen();
            SetWindowSize(windowedWidth, windowedHeight);
        } else {
            windowedWidth = GetScreenWidth();
            windowedHeight = GetScreenHeight();
            ToggleFullscreen();
        }
        GameApplyMonitorRefreshRate();
    }
    if (IsKeyPressed(KEY_F10)) {
        static unsigned int resolutionIndex = 0;
        if (IsWindowFullscreen()) ToggleFullscreen();
        ClearWindowState(FLAG_WINDOW_MAXIMIZED);
        const DisplayResolution resolution = DISPLAY_RESOLUTIONS[resolutionIndex];
        SetWindowSize(resolution.width, resolution.height);
        windowedWidth = resolution.width;
        windowedHeight = resolution.height;
        resolutionIndex = (resolutionIndex + 1u) % (sizeof(DISPLAY_RESOLUTIONS) / sizeof(DISPLAY_RESOLUTIONS[0]));
        GameApplyMonitorRefreshRate();
    }
}

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
        TraceLog(LOG_ERROR, "Could not load selected authored level %s; generating a fallback", choice->levelPath);
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
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_FULLSCREEN_MODE);
    int monitorWidth = GetMonitorWidth(0);
    int monitorHeight = GetMonitorHeight(0);
    InitWindow(monitorWidth, monitorHeight, "Slimeworks Factory");
    GameApplyMonitorRefreshRate();
    GameVisualsInit(&game->visuals);
    LevelCatalogRefresh(&game->levelCatalog);
}

void GameRun(Game *game)
{
    WorldSelector selector;
    WorldSelectorInit(&selector, &game->levelCatalog);
    while (!WindowShouldClose() && !selector.confirmed && !selector.cancelled) {
        GameHandleDisplayHotkeys();
        WorldSelectorUpdate(&selector);
        WorldSelectorDraw(&selector);
    }
    if (WindowShouldClose() || selector.cancelled) return;
    GameStartSession(game, &selector.choice);
    while (!WindowShouldClose()) {
        GameHandleDisplayHotkeys();
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
