#include "resource_node.h"
#include <math.h>

static Color ResourceColor(ResourceType type)
{
    switch (type) {
        case RESOURCE_GREEN_ORE: return (Color){ 79, 222, 128, 255 };
        case RESOURCE_ACID_ORE: return (Color){ 220, 229, 69, 255 };
        case RESOURCE_CRYSTAL_ORE: return (Color){ 183, 114, 255, 255 };
        default: return WHITE;
    }
}

static void AddNode(ResourceNodeSystem *system, Vector3 position, ResourceType resource, bool unlocked)
{
    if (system->count >= RESOURCE_NODE_MAX) return;
    system->nodes[system->count++] = (ResourceNode){
        .position = position, .resource = resource, .amount = 1.0f, .maxAmount = 1.0f,
        .respawnSeconds = 7.0f, .unlocked = unlocked
    };
}

static int AddNodesFromMarkers(ResourceNodeSystem *system, const Level *level, const char *tag, ResourceType resource, bool unlocked)
{
    Vector3 positions[RESOURCE_NODE_MAX];
    int count = LevelFindMarkersWithTag(level, tag, positions, RESOURCE_NODE_MAX);
    for (int i = 0; i < count && i < RESOURCE_NODE_MAX; ++i) AddNode(system, positions[i], resource, unlocked);
    return count;
}

void ResourceNodesInit(ResourceNodeSystem *system, const Level *level)
{
    *system = (ResourceNodeSystem){ 0 };
    if (!level || AddNodesFromMarkers(system, level, "ore_green", RESOURCE_GREEN_ORE, true) == 0) {
        AddNode(system, (Vector3){ -8, 0.45f, 8 }, RESOURCE_GREEN_ORE, true);
        AddNode(system, (Vector3){ 8, 0.45f, 8 }, RESOURCE_GREEN_ORE, true);
        AddNode(system, (Vector3){ -8, 0.45f, -8 }, RESOURCE_GREEN_ORE, true);
        AddNode(system, (Vector3){ 8, 0.45f, -8 }, RESOURCE_GREEN_ORE, true);
    }
    if (!level || AddNodesFromMarkers(system, level, "ore_acid", RESOURCE_ACID_ORE, false) == 0) {
        AddNode(system, (Vector3){ 34, 2.45f, 5 }, RESOURCE_ACID_ORE, false);
        AddNode(system, (Vector3){ 47, 2.45f, -6 }, RESOURCE_ACID_ORE, false);
    }
    if (!level || AddNodesFromMarkers(system, level, "ore_crystal", RESOURCE_CRYSTAL_ORE, false) == 0)
        AddNode(system, (Vector3){ 45, 5.45f, -40 }, RESOURCE_CRYSTAL_ORE, false);
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
    float reward = node->resource == RESOURCE_CRYSTAL_ORE ? 1.0f : 4.0f;
    EconomyAdd(economy, node->resource, reward);
    node->amount = 0.0f;
    node->respawnTimer = 0.0f;
    return true;
}

void ResourceNodesDraw(const ResourceNodeSystem *system)
{
    for (int i = 0; i < system->count; ++i) {
        const ResourceNode *node = &system->nodes[i];
        if (!node->unlocked) continue;
        Color color = ResourceColor(node->resource);
        if (node->amount <= 0.0f) color = Fade(DARKGRAY, 0.5f);
        DrawCylinder(node->position, 0.42f, 0.52f, 0.5f, 10, Fade(color, 0.75f));
        DrawSphere(node->position, 0.38f + 0.04f * sinf((float)GetTime() * 3.0f + i), color);
        DrawSphereWires(node->position, 0.41f, 8, 8, Fade(WHITE, 0.35f));
    }
}
