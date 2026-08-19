#ifndef SLIMEWORKS_RESOURCE_NODE_H
#define SLIMEWORKS_RESOURCE_NODE_H

#include <raylib.h>
#include "../economy/economy.h"
#include "../world/level.h"

#define RESOURCE_NODE_MAX 16

typedef struct ResourceNode {
    Vector3 position;
    ResourceType resource;
    float amount;
    float maxAmount;
    float respawnSeconds;
    float respawnTimer;
    bool unlocked;
} ResourceNode;

typedef struct ResourceNodeSystem {
    ResourceNode nodes[RESOURCE_NODE_MAX];
    int count;
} ResourceNodeSystem;

void ResourceNodesInit(ResourceNodeSystem *system);
void ResourceNodesAlignToLevel(ResourceNodeSystem *system, const Level *level);
void ResourceNodesUpdate(ResourceNodeSystem *system, float dt, bool zoneTwoUnlocked);
int ResourceNodesRaycast(const ResourceNodeSystem *system, Ray ray, float maxDistance, float *outDistance);
bool ResourceNodeHarvest(ResourceNode *node, Economy *economy);
void ResourceNodesDraw(const ResourceNodeSystem *system);

#endif
