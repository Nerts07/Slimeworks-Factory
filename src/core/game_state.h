#ifndef SLIMEWORKS_GAME_STATE_H
#define SLIMEWORKS_GAME_STATE_H

#include "../build/building.h"
#include "../contracts/contracts.h"
#include "../entities/resource_node.h"

typedef struct RareSlime {
    bool active;
    Vector3 position;
    float nextSpawnTimer;
} RareSlime;

typedef struct GameState {
    Economy economy;
    ResourceNodeSystem nodes;
    BuildingSystem buildings;
    ContractSystem contracts;
    bool zoneTwoUnlocked;
    float sessionSeconds;
    int rareSlimesCollected;
    Vector3 gatePosition;
    Vector3 rareSlimeSpawn;
    RareSlime rareSlime;
} GameState;

void GameStateInit(GameState *state, const Level *level);
bool GameStateUpdate(GameState *state, float dt);
bool GameStateTryUnlockZoneTwo(GameState *state);
bool GameStateCollectRareSlime(GameState *state, Ray ray);
void GameStateDrawWorld(const GameState *state, bool buildMode, BuildType selected, Vector3 previewPosition, bool previewValid);

#endif
