#ifndef SLIMEWORKS_GAME_H
#define SLIMEWORKS_GAME_H

#include "../gameplay/gameplay_system.h"
#include "../console/console.h"
#include "input_gate.h"
#include "../world/level.h"
#include "../world/level_catalog.h"
#include "update_loop.h"

typedef struct Game {
    Level level;
    LevelCatalog levelCatalog;
    GameState state;
    InputGate inputGate;
    PlayerSystem playerSystem;
    GameplaySystem gameplay;
    DeveloperConsole console;
    UpdateLoop updateLoop;
} Game;

void GameInit(Game *game);
void GameRun(Game *game);
void GameShutdown(Game *game);

#endif
