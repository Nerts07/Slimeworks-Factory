#ifndef SLIMEWORKS_GAMEPLAY_SYSTEM_H
#define SLIMEWORKS_GAMEPLAY_SYSTEM_H

#include "../core/game_state.h"
#include "../player/player_system.h"
#include "../world/level.h"
#include "gameplay_config.h"
#include "../core/input_gate.h"

typedef struct GameplaySystem {
    GameState *state;
    Level *level;
    PlayerSystem *playerSystem;
    const InputGate *inputGate;
    GameplayBindings bindings;
    GameplayRules rules;
    bool buildMode;
    bool helpOpen;
    BuildType selectedBuild;
    float harvestProgress;
    int targetedNode;
    char notification[96];
    float notificationSeconds;
} GameplaySystem;

void GameplaySystemInit(GameplaySystem *system, GameState *state, Level *level, PlayerSystem *playerSystem, const InputGate *inputGate);
void GameplaySystemUpdate(void *context, float deltaTime);
Vector3 GameplayBuildPreview(const GameplaySystem *system);
bool GameplayCanPreviewBuilding(const GameplaySystem *system, Vector3 position);
const ResourceNode *GameplayTargetedNode(const GameplaySystem *system);

#endif
