#include "resource_node.h"

#include <math.h>
#include <raymath.h>

typedef struct SlimeProfile {
    ResourceType resource;
    Color color;
    float harvestReward;
    bool requiresZoneTwo;
} SlimeProfile;

static const SlimeProfile SLIME_PROFILES[SLIME_TYPE_COUNT] = {
    [SLIME_TYPE_MOSSDROP] = { RESOURCE_GREEN_ORE, { 79, 222, 128, 255 }, 3.0f, false },
    [SLIME_TYPE_HONEYKIN] = { RESOURCE_GREEN_ORE, { 244, 190, 67, 255 }, 4.0f, false },
    [SLIME_TYPE_AZURE_PUFF] = { RESOURCE_GREEN_ORE, { 82, 181, 242, 255 }, 5.0f, false },
    [SLIME_TYPE_EMBERBUD] = { RESOURCE_GREEN_ORE, { 241, 111, 65, 255 }, 6.0f, false },
    [SLIME_TYPE_ACIDFLARE] = { RESOURCE_ACID_ORE, { 212, 232, 60, 255 }, 4.0f, true },
    [SLIME_TYPE_PRISMCORE] = { RESOURCE_CRYSTAL_ORE, { 183, 114, 255, 255 }, 2.0f, true },
    [SLIME_TYPE_AURORA_BLOOM] = { RESOURCE_CRYSTAL_ORE, { 79, 234, 205, 255 }, 3.0f, true },
    [SLIME_TYPE_CROWNSLIME] = { RESOURCE_CRYSTAL_ORE, { 255, 211, 72, 255 }, 5.0f, true }
};

static const SlimeProfile *ProfileFor(SlimeType type)
{
    return type >= 0 && type < SLIME_TYPE_COUNT ? &SLIME_PROFILES[type] : &SLIME_PROFILES[SLIME_TYPE_MOSSDROP];
}

static void AddNode(ResourceNodeSystem *system, Vector3 position, SlimeType slimeType)
{
    if (system->count >= RESOURCE_NODE_MAX) return;
    const SlimeProfile *profile = ProfileFor(slimeType);
    system->nodes[system->count++] = (ResourceNode){
        .position = position, .resource = profile->resource, .slimeType = slimeType,
        .amount = 1.0f, .maxAmount = 1.0f, .respawnSeconds = 7.0f, .unlocked = !profile->requiresZoneTwo
    };
}

static SlimeType LegacySlimeType(ResourceType resource)
{
    switch (resource) {
        case RESOURCE_ACID_ORE: return SLIME_TYPE_ACIDFLARE;
        case RESOURCE_CRYSTAL_ORE: return SLIME_TYPE_PRISMCORE;
        default: return SLIME_TYPE_MOSSDROP;
    }
}

static int AddNodesFromMarkers(ResourceNodeSystem *system, const Level *level, const char *tag, ResourceType resource)
{
    Vector3 positions[RESOURCE_NODE_MAX];
    int count = LevelFindMarkersWithTag(level, tag, positions, RESOURCE_NODE_MAX);
    for (int i = 0; i < count && i < RESOURCE_NODE_MAX; ++i) AddNode(system, positions[i], LegacySlimeType(resource));
    return count;
}

static bool IsVolumePositionFree(const ResourceNodeSystem *system, const Level *level, Vector3 position)
{
    if (LevelCollides(level, position.x, position.z, 0.7f)) return false;
    for (int i = 0; i < system->count; ++i)
        if (Vector3DistanceSqr(system->nodes[i].position, position) < 2.25f) return false;
    return true;
}

static void AddNodesFromVolume(ResourceNodeSystem *system, const Level *level, const LevelSlimeVolume *volume)
{
    Vector3 half = Vector3Scale(volume->size, 0.5f);
    Matrix rotation = MatrixRotateXYZ(Vector3Scale(volume->rotation, DEG2RAD));
    for (int spawned = 0; spawned < volume->slimeCount && system->count < RESOURCE_NODE_MAX; ++spawned) {
        Vector3 position = volume->center;
        bool placed = false;
        for (int attempt = 0; attempt < 20; ++attempt) {
            Vector3 local = { (float)GetRandomValue(-1000, 1000) * half.x / 1000.0f,
                              (float)GetRandomValue(-1000, 1000) * half.y / 1000.0f,
                              (float)GetRandomValue(-1000, 1000) * half.z / 1000.0f };
            Vector3 rotated = Vector3Transform(local, rotation);
            position.x = volume->center.x + rotated.x;
            position.z = volume->center.z + rotated.z;
            position.y = LevelGroundHeight(level, position.x, position.z) + 0.48f;
            if (IsVolumePositionFree(system, level, position)) { placed = true; break; }
        }
        if (placed) AddNode(system, position, volume->slimeType);
    }
}

static void AddNodesFromVolumes(ResourceNodeSystem *system, const Level *level)
{
    for (int volumeIndex = 0; volumeIndex < level->slimeVolumeCount && system->count < RESOURCE_NODE_MAX; ++volumeIndex) {
        AddNodesFromVolume(system, level, &level->slimeVolumes[volumeIndex]);
    }
}

void ResourceNodesInit(ResourceNodeSystem *system, const Level *level)
{
    *system = (ResourceNodeSystem){ 0 };
    if (level && level->slimeVolumeCount > 0) {
        AddNodesFromVolumes(system, level);
        return;
    }
    if (level) {
        int greenMarkers = AddNodesFromMarkers(system, level, "ore_green", RESOURCE_GREEN_ORE);
        int acidMarkers = AddNodesFromMarkers(system, level, "ore_acid", RESOURCE_ACID_ORE);
        int crystalMarkers = AddNodesFromMarkers(system, level, "ore_crystal", RESOURCE_CRYSTAL_ORE);
        if (greenMarkers + acidMarkers + crystalMarkers > 0) return;

        const LevelSlimeVolume fallbackVolumes[] = {
            { .center = { -9.0f, 0.0f, 8.0f }, .size = { 16.0f, 5.0f, 14.0f }, .slimeCount = 2, .slimeType = SLIME_TYPE_MOSSDROP },
            { .center = { 9.0f, 0.0f, 8.0f }, .size = { 16.0f, 5.0f, 14.0f }, .slimeCount = 2, .slimeType = SLIME_TYPE_HONEYKIN },
            { .center = { -9.0f, 0.0f, -8.0f }, .size = { 16.0f, 5.0f, 14.0f }, .slimeCount = 2, .slimeType = SLIME_TYPE_AZURE_PUFF },
            { .center = { 9.0f, 0.0f, -8.0f }, .size = { 16.0f, 5.0f, 14.0f }, .slimeCount = 2, .slimeType = SLIME_TYPE_EMBERBUD }
        };
        for (int i = 0; i < (int)(sizeof(fallbackVolumes) / sizeof(fallbackVolumes[0])); ++i)
            AddNodesFromVolume(system, level, &fallbackVolumes[i]);
        return;
    }
    AddNode(system, (Vector3){ -8, 0.45f, 8 }, SLIME_TYPE_MOSSDROP);
    AddNode(system, (Vector3){ 8, 0.45f, 8 }, SLIME_TYPE_HONEYKIN);
    AddNode(system, (Vector3){ -8, 0.45f, -8 }, SLIME_TYPE_AZURE_PUFF);
    AddNode(system, (Vector3){ 8, 0.45f, -8 }, SLIME_TYPE_EMBERBUD);
}

void ResourceNodesAlignToLevel(ResourceNodeSystem *system, const Level *level)
{
    for (int i = 0; i < system->count; ++i) {
        ResourceNode *node = &system->nodes[i];
        node->position.y = LevelGroundHeight(level, node->position.x, node->position.z) + 0.48f;
    }
}

void ResourceNodesUpdate(ResourceNodeSystem *system, float dt, bool zoneTwoUnlocked)
{
    for (int i = 0; i < system->count; ++i) {
        ResourceNode *node = &system->nodes[i];
        if (!node->unlocked && zoneTwoUnlocked) node->unlocked = true;
        if (!node->unlocked || node->amount > 0.0f) continue;
        node->respawnTimer += dt;
        if (node->respawnTimer >= node->respawnSeconds) {
            node->amount = node->maxAmount;
            node->respawnTimer = 0.0f;
        }
    }
}

int ResourceNodesRaycast(const ResourceNodeSystem *system, Ray ray, float maxDistance, float *outDistance)
{
    int found = -1;
    float best = maxDistance;
    for (int i = 0; i < system->count; ++i) {
        const ResourceNode *node = &system->nodes[i];
        if (!node->unlocked || node->amount <= 0.0f) continue;
        RayCollision hit = GetRayCollisionSphere(ray, node->position, 0.55f);
        if (hit.hit && hit.distance < best) { found = i; best = hit.distance; }
    }
    if (outDistance) *outDistance = best;
    return found;
}

bool ResourceNodeHarvest(ResourceNode *node, Economy *economy)
{
    if (!node->unlocked || node->amount <= 0.0f) return false;
    EconomyAdd(economy, node->resource, ProfileFor(node->slimeType)->harvestReward);
    node->amount = 0.0f;
    node->respawnTimer = 0.0f;
    return true;
}

static GameVisualKind ResourceVisual(ResourceType type)
{
    switch (type) {
        case RESOURCE_GREEN_ORE: return GAME_VISUAL_ORE_GREEN;
        case RESOURCE_ACID_ORE: return GAME_VISUAL_ORE_ACID;
        case RESOURCE_CRYSTAL_ORE: return GAME_VISUAL_ORE_CRYSTAL;
        default: return GAME_VISUAL_ORE_GREEN;
    }
}

void ResourceNodesDraw(const ResourceNodeSystem *system, const GameVisuals *visuals)
{
    for (int i = 0; i < system->count; ++i) {
        const ResourceNode *node = &system->nodes[i];
        if (!node->unlocked) continue;
        Color color = ProfileFor(node->slimeType)->color;
        if (node->amount <= 0.0f) color = Fade(DARKGRAY, 0.5f);
        if (!GameVisualsDraw(visuals, ResourceVisual(node->resource), node->position, 0.0f, (Vector3){ 1, 1, 1 }, color)) {
            DrawCylinder(node->position, 0.42f, 0.52f, 0.5f, 10, Fade(color, 0.75f));
            DrawSphere(node->position, 0.38f + 0.04f * sinf((float)GetTime() * 3.0f + i), color);
            DrawSphereWires(node->position, 0.41f, 8, 8, Fade(WHITE, 0.35f));
        }
    }
}
