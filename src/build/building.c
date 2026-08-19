#include "building.h"
#include <math.h>
#include <stddef.h>
#include <raymath.h>

static float DistanceXZ(Vector3 a, Vector3 b)
{
    float dx = a.x - b.x, dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

static bool IsAdjacent(Vector3 a, Vector3 b)
{
    float dx = fabsf(a.x - b.x), dz = fabsf(a.z - b.z);
    /* Grid corners connect too: hand-placing pipes stays forgiving. */
    return dx <= 1.1f && dz <= 1.1f && (dx > 0.1f || dz > 0.1f);
}

void BuildingsInit(BuildingSystem *system)
{
    *system = (BuildingSystem){ 0 };
}

const char *BuildingName(BuildType type)
{
    static const char *names[BUILD_COUNT] = { "Pump", "Refinery", "Acid mixer", "Booster", "Pipe" };
    return (type >= 0 && type < BUILD_COUNT) ? names[type] : "Unknown";
}

const char *BuildingStatusText(BuildingStatus status)
{
    static const char *texts[] = {
        "Working", "Connect a pipe", "No receiver on this network",
        "Waiting for input", "Receiver buffer full", "Place near an ore node"
    };
    return (status >= BUILDING_WORKING && status <= BUILDING_NEEDS_SOURCE) ? texts[status] : "Unknown";
}

float BuildingCost(BuildType type)
{
    static const float costs[BUILD_COUNT] = { 8.0f, 10.0f, 16.0f, 12.0f, 1.0f };
    return (type >= 0 && type < BUILD_COUNT) ? costs[type] : 9999.0f;
}

static bool IsBuildArea(Vector3 p)
{
    return p.x >= -58.0f && p.x <= 58.0f && p.z >= -58.0f && p.z <= 58.0f;
}

static bool TypeConsumes(BuildType type, ResourceType resource)
{
    return (type == BUILD_REFINERY && resource == RESOURCE_GREEN_ORE) ||
           (type == BUILD_MIXER && (resource == RESOURCE_GREEN_ORE || resource == RESOURCE_ACID_ORE));
}

static bool HasPipeNeighbor(const BuildingSystem *system, int index)
{
    for (int i = 0; i < system->count; ++i)
        if (system->items[i].type == BUILD_PIPE && IsAdjacent(system->items[i].position, system->items[index].position)) return true;
    return false;
}

static int FindConsumerThroughPipes(const BuildingSystem *system, int sourceIndex, ResourceType resource,
                                    int routingCursor, bool visited[BUILDING_MAX], int parent[BUILDING_MAX],
                                    bool *hasConsumer)
{
    int queue[BUILDING_MAX];
    int available[BUILDING_MAX];
    int head = 0, tail = 0;
    int availableCount = 0;
    bool consumerSeen[BUILDING_MAX] = { false };
    for (int i = 0; i < BUILDING_MAX; ++i) parent[i] = -1;
    *hasConsumer = false;
    queue[tail++] = sourceIndex;
    visited[sourceIndex] = true;
    while (head < tail) {
        int current = queue[head++];
        for (int i = 0; i < system->count; ++i) {
            if (visited[i] || !IsAdjacent(system->items[current].position, system->items[i].position)) continue;
            const Building *next = &system->items[i];
            if (next->type == BUILD_PIPE) {
                visited[i] = true;
                parent[i] = current;
                queue[tail++] = i;
                continue;
            }
            if (current == sourceIndex || !TypeConsumes(next->type, resource) || consumerSeen[i]) continue;
            consumerSeen[i] = true;
            parent[i] = current;
            *hasConsumer = true;
            if (next->buffer[resource] < BUILDING_BUFFER_CAPACITY - 0.01f) available[availableCount++] = i;
        }
    }
    return availableCount > 0 ? available[routingCursor % availableCount] : -1;
}

static void MarkFlowPath(BuildingSystem *system, int sourceIndex, int consumerIndex,
                         const int parent[BUILDING_MAX], ResourceType resource)
{
    int downstream = consumerIndex;
    while (downstream != sourceIndex && parent[downstream] >= 0) {
        int upstream = parent[downstream];
        if (system->items[upstream].type == BUILD_PIPE) {
            Vector2 direction = {
                system->items[downstream].position.x - system->items[upstream].position.x,
                system->items[downstream].position.z - system->items[upstream].position.z
            };
            if (Vector2Length(direction) > 0.001f) direction = Vector2Normalize(direction);
            system->items[upstream].flowTimer = 0.65f;
            system->items[upstream].flowResource = resource;
            system->items[upstream].flowDirection = direction;
        }
        downstream = upstream;
    }
}

static void TransferFromPump(BuildingSystem *system, int pumpIndex, ResourceType resource, float amount)
{
    Building *pump = &system->items[pumpIndex];
    bool visited[BUILDING_MAX] = { false };
    int parent[BUILDING_MAX];
    bool hasConsumer = false;
    int routingCursor = (int)pump->progress;
    int consumer = FindConsumerThroughPipes(system, pumpIndex, resource, routingCursor, visited, parent, &hasConsumer);
    if (consumer < 0) {
        pump->status = hasConsumer ? BUILDING_OUTPUT_FULL : BUILDING_NEEDS_DESTINATION;
        return;
    }
    float moved = fminf(amount, pump->buffer[resource]);
    moved = fminf(moved, BUILDING_BUFFER_CAPACITY - system->items[consumer].buffer[resource]);
    if (moved <= 0.0f) { pump->status = BUILDING_OUTPUT_FULL; return; }
    pump->buffer[resource] -= moved;
    system->items[consumer].buffer[resource] += moved;
    pump->progress = (float)((routingCursor + 1) % BUILDING_MAX);
    pump->status = BUILDING_WORKING;
    MarkFlowPath(system, pumpIndex, consumer, parent, resource);
}

bool BuildingsTryPlace(BuildingSystem *system, Economy *economy, BuildType type, Vector3 position, ResourceType sourceResource)
{
    if (system->count >= BUILDING_MAX || !IsBuildArea(position) || !EconomyCanAfford(economy, RESOURCE_GREEN_ORE, BuildingCost(type))) return false;
    if (type == BUILD_PUMP && (sourceResource < 0 || sourceResource >= RESOURCE_COUNT)) return false;
    for (int i = 0; i < system->count; ++i) if (DistanceXZ(system->items[i].position, position) < 0.7f) return false;
    EconomySpend(economy, RESOURCE_GREEN_ORE, BuildingCost(type));
    system->items[system->count++] = (Building){
        .type = type, .position = position, .level = 1, .sourceResource = sourceResource,
        .flowResource = RESOURCE_GREEN_ORE,
        .status = (type == BUILD_PIPE || type == BUILD_BOOSTER) ? BUILDING_WORKING : BUILDING_NEEDS_PIPE
    };
    return true;
}

bool BuildingsTryRemovePipe(BuildingSystem *system, Economy *economy, Vector3 position)
{
    int best = -1;
    float bestDistance = 1.1f;
    for (int i = 0; i < system->count; ++i) if (system->items[i].type == BUILD_PIPE) {
        float distance = DistanceXZ(system->items[i].position, position);
        if (distance < bestDistance) { bestDistance = distance; best = i; }
    }
    if (best < 0) return false;
    system->items[best] = system->items[--system->count];
    EconomyAdd(economy, RESOURCE_GREEN_ORE, 0.5f);
    return true;
}

BuildingUpgradeResult BuildingsTryUpgrade(BuildingSystem *system, Economy *economy, Vector3 position, float maxDistance)
{
    int best = -1;
    float distance = maxDistance;
    for (int i = 0; i < system->count; ++i) {
        if (system->items[i].type == BUILD_PIPE) continue;
        float candidate = DistanceXZ(system->items[i].position, position);
        if (candidate < distance) { distance = candidate; best = i; }
    }
    if (best < 0) return BUILDING_UPGRADE_NO_STATION;
    if (system->items[best].level >= 2) return BUILDING_UPGRADE_MAX_LEVEL;
    if (!EconomySpend(economy, RESOURCE_SLIME, 14.0f)) return BUILDING_UPGRADE_NOT_ENOUGH_SLIME;
    system->items[best].level++;
    return BUILDING_UPGRADE_OK;
}

void BuildingsUpdate(BuildingSystem *system, Economy *economy, float dt)
{
    for (int i = 0; i < system->count; ++i) {
        Building *building = &system->items[i];
        building->flowTimer = fmaxf(0.0f, building->flowTimer - dt);
        if (building->type == BUILD_PIPE || building->type == BUILD_BOOSTER) continue;
        float multiplier = (float)building->level;
        for (int b = 0; b < system->count; ++b)
            if (system->items[b].type == BUILD_BOOSTER && DistanceXZ(system->items[b].position, building->position) <= 3.2f) multiplier += 0.35f * system->items[b].level;

        if (building->type == BUILD_PUMP) {
            if (!HasPipeNeighbor(system, i)) { building->status = BUILDING_NEEDS_PIPE; continue; }
            float room = BUILDING_BUFFER_CAPACITY - building->buffer[building->sourceResource];
            if (room > 0.01f) building->buffer[building->sourceResource] += fminf(room, 1.6f * multiplier * dt);
            /* A full pump must still try to transfer. This lets a network recover after pipes are connected later. */
            TransferFromPump(system, i, building->sourceResource, 2.8f * multiplier * dt);
        } else if (building->type == BUILD_REFINERY) {
            if (!HasPipeNeighbor(system, i)) { building->status = BUILDING_NEEDS_PIPE; continue; }
            if (building->buffer[RESOURCE_GREEN_ORE] <= 0.01f) { building->status = BUILDING_NEEDS_INPUT; continue; }
            float consumed = fminf(building->buffer[RESOURCE_GREEN_ORE], 1.4f * multiplier * dt);
            building->buffer[RESOURCE_GREEN_ORE] -= consumed;
            EconomyAdd(economy, RESOURCE_SLIME, consumed * 1.8f);
            building->progress += consumed;
            building->status = BUILDING_WORKING;
        } else if (building->type == BUILD_MIXER) {
            if (!HasPipeNeighbor(system, i)) { building->status = BUILDING_NEEDS_PIPE; continue; }
            float mix = fminf(building->buffer[RESOURCE_GREEN_ORE] * 0.5f, building->buffer[RESOURCE_ACID_ORE]);
            if (mix <= 0.01f) { building->status = BUILDING_NEEDS_INPUT; continue; }
            float consumedAcid = fminf(mix, 0.75f * multiplier * dt);
            building->buffer[RESOURCE_ACID_ORE] -= consumedAcid;
            building->buffer[RESOURCE_GREEN_ORE] -= consumedAcid * 2.0f;
            EconomyAdd(economy, RESOURCE_SLIME, consumedAcid * 6.0f);
            building->progress += consumedAcid;
            building->status = BUILDING_WORKING;
        }
    }
}

int BuildingsCountType(const BuildingSystem *system, BuildType type)
{
    int count = 0;
    for (int i = 0; i < system->count; ++i) if (system->items[i].type == type) count++;
    return count;
}

bool BuildingsHasPumpFor(const BuildingSystem *system, ResourceType resource)
{
    for (int i = 0; i < system->count; ++i)
        if (system->items[i].type == BUILD_PUMP && system->items[i].sourceResource == resource) return true;
    return false;
}

const Building *BuildingsFindNearest(const BuildingSystem *system, Vector3 position, float maxDistance)
{
    const Building *best = NULL;
    for (int i = 0; i < system->count; ++i) {
        float distance = DistanceXZ(system->items[i].position, position);
        if (distance <= maxDistance) { maxDistance = distance; best = &system->items[i]; }
    }
    return best;
}

static Color BuildingColor(BuildType type)
{
    switch (type) {
        case BUILD_PUMP: return (Color){ 72, 178, 112, 255 };
        case BUILD_REFINERY: return (Color){ 75, 152, 218, 255 };
        case BUILD_MIXER: return (Color){ 181, 104, 218, 255 };
        case BUILD_BOOSTER: return (Color){ 242, 169, 67, 255 };
        case BUILD_PIPE: return (Color){ 104, 116, 130, 255 };
        default: return WHITE;
    }
}

static Color ResourceColor(ResourceType type)
{
    static const Color colors[RESOURCE_COUNT] = { LIME, YELLOW, VIOLET, SKYBLUE };
    return (type >= 0 && type < RESOURCE_COUNT) ? colors[type] : WHITE;
}

static GameVisualKind BuildingVisualKind(BuildType type)
{
    static const GameVisualKind kinds[BUILD_COUNT] = {
        GAME_VISUAL_BUILD_PUMP, GAME_VISUAL_BUILD_REFINERY, GAME_VISUAL_BUILD_MIXER,
        GAME_VISUAL_BUILD_BOOSTER, GAME_VISUAL_BUILD_PIPE
    };
    return (type >= 0 && type < BUILD_COUNT) ? kinds[type] : GAME_VISUAL_BUILD_PUMP;
}

void BuildingsDraw(const BuildingSystem *system, const GameVisuals *visuals, bool buildMode, BuildType selected,
                   Vector3 previewPosition, bool previewValid)
{
    for (int i = 0; i < system->count; ++i) for (int j = i + 1; j < system->count; ++j) {
        const Building *a = &system->items[i], *b = &system->items[j];
        if ((a->type == BUILD_PIPE || b->type == BUILD_PIPE) && IsAdjacent(a->position, b->position)) {
            Color line = (a->flowTimer > 0.0f || b->flowTimer > 0.0f) ? LIME : Fade(SKYBLUE, 0.45f);
            DrawLine3D((Vector3){ a->position.x, a->position.y + 0.32f, a->position.z },
                       (Vector3){ b->position.x, b->position.y + 0.32f, b->position.z }, line);
        }
    }
    for (int i = 0; i < system->count; ++i) {
        const Building *b = &system->items[i];
        if (b->type == BUILD_PIPE) {
            Vector2 modelDirection = Vector2Length(b->flowDirection) > 0.001f ? b->flowDirection : (Vector2){ 0.0f, 1.0f };
            float rotationY = atan2f(modelDirection.x, modelDirection.y) * RAD2DEG;
            if (!GameVisualsDraw(visuals, GAME_VISUAL_BUILD_PIPE, b->position, rotationY, (Vector3){ 1, 1, 1 }, WHITE))
                DrawCube((Vector3){ b->position.x, b->position.y + 0.14f, b->position.z }, 0.82f, 0.18f, 0.82f, BuildingColor(b->type));
            if (b->flowTimer > 0.0f) {
                float phase = fmodf((float)GetTime() * 4.0f + b->position.x + b->position.z, 1.0f) - 0.5f;
                Vector2 direction = Vector2Length(b->flowDirection) > 0.001f ? b->flowDirection : (Vector2){ 1.0f, 0.0f };
                DrawSphere((Vector3){ b->position.x + direction.x * phase * 0.72f, b->position.y + 0.32f,
                                      b->position.z + direction.y * phase * 0.72f }, 0.14f, ResourceColor(b->flowResource));
            }
            continue;
        }
        Color color = BuildingColor(b->type);
        bool modelDrawn = GameVisualsDraw(visuals, BuildingVisualKind(b->type), b->position, 0.0f, (Vector3){ 1, 1, 1 }, WHITE);
        if (!modelDrawn) {
            DrawCylinder((Vector3){ b->position.x, b->position.y + 0.15f, b->position.z }, 0.62f, 0.62f, 0.3f, 16, Fade(DARKGRAY, 0.9f));
            DrawCube((Vector3){ b->position.x, b->position.y + 0.62f, b->position.z }, 0.92f, 0.94f, 0.92f, color);
            DrawCubeWires((Vector3){ b->position.x, b->position.y + 0.62f, b->position.z }, 0.96f, 0.98f, 0.96f, Fade(WHITE, 0.5f));
            DrawCylinder((Vector3){ b->position.x, b->position.y + 1.26f, b->position.z }, 0.26f, 0.4f, 0.34f, b->type == BUILD_MIXER ? 6 : 12, Fade(WHITE, 0.8f));
        }
        if (b->level > 1) DrawSphere((Vector3){ b->position.x, b->position.y + 1.65f, b->position.z }, 0.14f, GOLD);
        if (b->status != BUILDING_WORKING) {
            float pulse = 0.12f + 0.025f * sinf((float)GetTime() * 5.0f);
            DrawSphere((Vector3){ b->position.x, b->position.y + 1.78f, b->position.z }, pulse, b->status == BUILDING_OUTPUT_FULL ? ORANGE : RED);
        }
    }
    if (buildMode) {
        Color preview = previewValid ? Fade(BuildingColor(selected), 0.55f) : Fade(RED, 0.55f);
        float height = selected == BUILD_PIPE ? 0.16f : 0.8f;
        if (!GameVisualsDraw(visuals, BuildingVisualKind(selected), previewPosition, 0.0f, (Vector3){ 1, 1, 1 }, preview))
            DrawCube((Vector3){ previewPosition.x, previewPosition.y + height * 0.5f, previewPosition.z }, 0.85f, height, 0.85f, preview);
        DrawCubeWires((Vector3){ previewPosition.x, previewPosition.y + height * 0.5f, previewPosition.z }, 0.9f, height + 0.04f, 0.9f, previewValid ? LIME : RED);
    }
}
