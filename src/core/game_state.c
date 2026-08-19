#include "game_state.h"
#include <raymath.h>

static const float ZONE_TWO_COST = 55.0f;

void GameStateInit(GameState *state, const Level *level)
{
    *state = (GameState){ 0 };
    EconomyInit(&state->economy);
    ResourceNodesInit(&state->nodes, level);
    BuildingsInit(&state->buildings);
    ContractsInit(&state->contracts);
    state->gatePosition = (Vector3){ 20.0f, 0.0f, -3.0f };
    state->rareSlimeSpawn = (Vector3){ 38.0f, 0.0f, -43.0f };
    if (level) {
        LevelFindMarker(level, "zone_two_gate", &state->gatePosition);
        LevelFindMarker(level, "rare_slime_spawn", &state->rareSlimeSpawn);
        state->gatePosition.y = LevelGroundHeight(level, state->gatePosition.x, state->gatePosition.z);
        state->rareSlimeSpawn.y = LevelGroundHeight(level, state->rareSlimeSpawn.x, state->rareSlimeSpawn.z) + 0.6f;
    }
    state->rareSlime.nextSpawnTimer = 35.0f;
}

bool GameStateUpdate(GameState *state, float dt)
{
    state->sessionSeconds += dt;
    BuildingsUpdate(&state->buildings, &state->economy, dt);
    ResourceNodesUpdate(&state->nodes, dt, state->zoneTwoUnlocked);
    if (!state->rareSlime.active) {
        state->rareSlime.nextSpawnTimer -= dt;
        if (state->rareSlime.nextSpawnTimer <= 0.0f && state->zoneTwoUnlocked) {
            state->rareSlime.active = true;
            state->rareSlime.position = state->rareSlimeSpawn;
        }
    }
    return ContractsUpdate(&state->contracts, &state->economy, &state->buildings, state->zoneTwoUnlocked);
}

bool GameStateTryUnlockZoneTwo(GameState *state)
{
    if (state->zoneTwoUnlocked || !EconomySpend(&state->economy, RESOURCE_SLIME, ZONE_TWO_COST)) return false;
    state->zoneTwoUnlocked = true;
    return true;
}

bool GameStateCollectRareSlime(GameState *state, Ray ray)
{
    if (!state->rareSlime.active) return false;
    RayCollision hit = GetRayCollisionSphere(ray, state->rareSlime.position, 0.6f);
    if (!hit.hit || hit.distance > 4.0f) return false;
    state->rareSlime.active = false;
    state->rareSlime.nextSpawnTimer = 45.0f;
    state->rareSlimesCollected++;
    EconomyAdd(&state->economy, RESOURCE_SLIME, 28.0f);
    EconomyAdd(&state->economy, RESOURCE_CRYSTAL_ORE, 2.0f);
    return true;
}

void GameStateDrawWorld(const GameState *state, const GameVisuals *visuals, bool buildMode, BuildType selected,
                        Vector3 previewPosition, bool previewValid)
{
    ResourceNodesDraw(&state->nodes, visuals);
    BuildingsDraw(&state->buildings, visuals, buildMode, selected, previewPosition, previewValid);

    DrawCylinderWires((Vector3){ 0.0f, 0.09f, 0.0f }, 6.1f, 6.1f, 0.04f, 48, Fade(SKYBLUE, 0.55f));
    Vector3 objective = { 0.0f, 1.8f, 0.0f };
    if (state->contracts.step == TUTORIAL_HARVEST_GREEN || state->contracts.step == TUTORIAL_PLACE_PUMP)
        objective = state->nodes.nodes[0].position;
    else if (state->contracts.step == TUTORIAL_UNLOCK_ACID) objective = state->gatePosition;
    else if (state->contracts.step == TUTORIAL_PLACE_MIXER) {
        for (int i = 0; i < state->nodes.count; ++i) {
            if (state->nodes.nodes[i].resource == RESOURCE_ACID_ORE) { objective = state->nodes.nodes[i].position; break; }
        }
    }
    DrawCylinderWires((Vector3){ objective.x, objective.y + 1.2f, objective.z }, 0.38f, 0.38f, 1.2f, 10, GOLD);
    DrawSphere((Vector3){ objective.x, objective.y + 2.0f + 0.15f*sinf((float)GetTime()*4.0f), objective.z }, 0.16f, GOLD);
    if (state->rareSlime.active) {
        float pulse = 0.52f + 0.09f * sinf((float)GetTime() * 5.0f);
        if (!GameVisualsDraw(visuals, GAME_VISUAL_RARE_SLIME, state->rareSlime.position, 0.0f,
                             (Vector3){ 1, 1, 1 }, WHITE)) {
            DrawSphere(state->rareSlime.position, pulse, VIOLET);
            DrawSphereWires(state->rareSlime.position, pulse + 0.07f, 10, 10, GOLD);
        }
    }
}
