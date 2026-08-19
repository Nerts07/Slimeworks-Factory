#include "gameplay_system.h"
#include "../save/save.h"
#include <math.h>
#include <raymath.h>
#include <stdio.h>

static void Notify(GameplaySystem *system, const char *message)
{
    snprintf(system->notification, sizeof(system->notification), "%s", message);
    system->notificationSeconds = 2.2f;
}

static Ray GameplayRay(const GameplaySystem *system)
{
    return GetScreenToWorldRay((Vector2){ GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f }, system->playerSystem->camera);
}

Vector3 GameplayBuildPreview(const GameplaySystem *system)
{
    const Player *player = &system->playerSystem->player;
    Vector3 look = PlayerLookDirection(player);
    Vector2 horizontal = Vector2Normalize((Vector2){ look.x, look.z });
    Vector3 result = { player->position.x + horizontal.x * 3.0f, 0.0f, player->position.z + horizontal.y * 3.0f };
    result.x = roundf(result.x);
    result.z = roundf(result.z);
    if (system->selectedBuild == BUILD_PIPE) {
        int closest = -1;
        float closestDistance = 3.5f;
        for (int i = 0; i < system->state->buildings.count; ++i) {
            Vector3 anchor = system->state->buildings.items[i].position;
            float dx = result.x - anchor.x, dz = result.z - anchor.z;
            float distance = sqrtf(dx * dx + dz * dz);
            if (distance < 0.1f || distance >= closestDistance) continue;
            closest = i;
            closestDistance = distance;
        }
        if (closest >= 0) {
            Vector3 anchor = system->state->buildings.items[closest].position;
            float dx = result.x - anchor.x, dz = result.z - anchor.z;
            if (fabsf(dx) >= fabsf(dz)) { result.x = anchor.x + (dx >= 0.0f ? 1.0f : -1.0f); result.z = anchor.z; }
            else { result.x = anchor.x; result.z = anchor.z + (dz >= 0.0f ? 1.0f : -1.0f); }
        }
    }
    result.y = LevelGroundHeight(system->level, result.x, result.z);
    return result;
}

bool GameplayCanPreviewBuilding(const GameplaySystem *system, Vector3 position)
{
    float limit = system->rules.buildWorldLimit;
    if (position.x < -limit || position.x > limit || position.z < -limit || position.z > limit) return false;
    if (LevelSampleGround(system->level, position.x, position.z).type != GROUND_FLAT) return false;
    if (!ContractsAllowsBuild(&system->state->contracts, system->selectedBuild)) return false;
    if (!EconomyCanAfford(&system->state->economy, RESOURCE_GREEN_ORE, BuildingCost(system->selectedBuild))) return false;
    for (int i = 0; i < system->state->buildings.count; ++i) {
        Vector3 p = system->state->buildings.items[i].position;
        float dx = p.x - position.x, dz = p.z - position.z;
        if (dx * dx + dz * dz < 0.7f * 0.7f) return false;
    }
    return !LevelCollides(system->level, position.x, position.z, 0.65f);
}

static ResourceType NearestNodeResource(const GameplaySystem *system, Vector3 position)
{
    ResourceType found = RESOURCE_COUNT;
    float best = system->rules.pumpSourceRadius;
    for (int i = 0; i < system->state->nodes.count; ++i) {
        const ResourceNode *node = &system->state->nodes.nodes[i];
        if (!node->unlocked) continue;
        float dx = node->position.x - position.x, dz = node->position.z - position.z;
        float distance = sqrtf(dx * dx + dz * dz);
        if (distance < best) { best = distance; found = node->resource; }
    }
    return found;
}

static void UpdateBuildInput(GameplaySystem *system)
{
    if (IsKeyPressed(system->bindings.buildPump)) system->selectedBuild = BUILD_PUMP;
    if (IsKeyPressed(system->bindings.buildRefinery)) system->selectedBuild = BUILD_REFINERY;
    if (IsKeyPressed(system->bindings.buildMixer)) system->selectedBuild = BUILD_MIXER;
    if (IsKeyPressed(system->bindings.buildBooster)) system->selectedBuild = BUILD_BOOSTER;
    if (IsKeyPressed(system->bindings.buildPipe)) system->selectedBuild = BUILD_PIPE;
    Vector3 preview = GameplayBuildPreview(system);
    if (IsKeyPressed(system->bindings.placeOrInteract)) {
        ResourceType source = NearestNodeResource(system, preview);
        if (!ContractsAllowsBuild(&system->state->contracts, system->selectedBuild)) Notify(system, "Complete the current contract first");
        else if (system->selectedBuild == BUILD_PUMP && source == RESOURCE_COUNT) Notify(system, "Pump must be beside an ore node");
        else if (GameplayCanPreviewBuilding(system, preview) && BuildingsTryPlace(&system->state->buildings, &system->state->economy, system->selectedBuild, preview, source))
            Notify(system, system->selectedBuild == BUILD_PIPE ? "Pipe placed: blue line means connected" : "Built");
        else Notify(system, "Cannot build here or not enough ore");
    }
    if (IsKeyPressed(system->bindings.removePipe)) Notify(system, BuildingsTryRemovePipe(&system->state->buildings, &system->state->economy, preview) ? "Pipe removed (+0.5 ore)" : "No pipe nearby");
    if (IsKeyPressed(system->bindings.upgrade)) Notify(system, BuildingsTryUpgrade(&system->state->buildings, &system->state->economy, preview) ? "Station upgraded" : "Need 14 slime or a nearby level 1 station");
}

static void UpdateWorldInteraction(GameplaySystem *system, float deltaTime)
{
    Ray ray = GameplayRay(system);
    float hitDistance;
    system->targetedNode = ResourceNodesRaycast(&system->state->nodes, ray, system->rules.interactionRange, &hitDistance);
    if (IsKeyPressed(system->bindings.placeOrInteract) && GameStateCollectRareSlime(system->state, ray)) {
        system->harvestProgress = 0.0f;
        Notify(system, "Rare crystal slime captured! +28 slime");
    } else if (system->targetedNode >= 0 && IsKeyDown(system->bindings.placeOrInteract)) {
        system->harvestProgress += deltaTime / 0.55f;
        if (system->harvestProgress >= 1.0f) {
            ResourceNodeHarvest(&system->state->nodes.nodes[system->targetedNode], &system->state->economy);
            system->harvestProgress = 0.0f;
            Notify(system, "Ore collected");
        }
    } else system->harvestProgress = 0.0f;
}

void GameplaySystemInit(GameplaySystem *system, GameState *state, Level *level, PlayerSystem *playerSystem, const InputGate *inputGate)
{
    *system = (GameplaySystem){ .state = state, .level = level, .playerSystem = playerSystem,
                                .inputGate = inputGate,
                                .bindings = GameplayDefaultBindings(), .rules = GameplayDefaultRules(),
                                .selectedBuild = BUILD_PUMP, .targetedNode = -1 };
    Vector3 gate;
    if (LevelFindMarker(level, "zone_two_gate", &gate)) system->rules.gatePosition = gate;
}

void GameplaySystemUpdate(void *context, float deltaTime)
{
    GameplaySystem *system = context;
    if (system->notificationSeconds > 0.0f) system->notificationSeconds -= deltaTime;
    system->level->zoneTwoUnlocked = system->state->zoneTwoUnlocked;
    if (GameStateUpdate(system->state, deltaTime)) Notify(system, ContractsObjective(&system->state->contracts));

    if (system->inputGate && system->inputGate->gameplayBlocked) return;

    if (IsKeyPressed(system->bindings.save)) Notify(system, SaveGameWrite(system->state) ? "Progress saved" : "Could not save progress");
    if (IsKeyPressed(system->bindings.load)) Notify(system, SaveGameRead(system->state) ? "Progress loaded" : "No valid save found");
    if (IsKeyPressed(system->bindings.toggleHelp)) system->helpOpen = !system->helpOpen;
    if (IsKeyPressed(system->bindings.toggleBuild)) { system->buildMode = !system->buildMode; system->harvestProgress = 0.0f; }
    if (system->buildMode) { UpdateBuildInput(system); system->targetedNode = -1; }
    else UpdateWorldInteraction(system, deltaTime);

    if (!system->state->zoneTwoUnlocked && IsKeyPressed(system->bindings.unlockZone)) {
        Vector3 p = system->playerSystem->player.position;
        float dx = p.x - system->rules.gatePosition.x, dz = p.z - system->rules.gatePosition.z;
        if (dx * dx + dz * dz < system->rules.gateInteractionRadius * system->rules.gateInteractionRadius)
            Notify(system, GameStateTryUnlockZoneTwo(system->state) ? "Zone 2 unlocked!" : "Need 55 slime to unlock Zone 2");
    }
}

const ResourceNode *GameplayTargetedNode(const GameplaySystem *system)
{
    return system->targetedNode >= 0 ? &system->state->nodes.nodes[system->targetedNode] : NULL;
}
