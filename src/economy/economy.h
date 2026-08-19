#ifndef SLIMEWORKS_ECONOMY_H
#define SLIMEWORKS_ECONOMY_H

#include <stdbool.h>

typedef enum ResourceType {
    RESOURCE_GREEN_ORE,
    RESOURCE_ACID_ORE,
    RESOURCE_CRYSTAL_ORE,
    RESOURCE_SLIME,
    RESOURCE_COUNT
} ResourceType;

typedef struct Economy {
    float amount[RESOURCE_COUNT];
} Economy;

void EconomyInit(Economy *economy);
void EconomyAdd(Economy *economy, ResourceType type, float amount);
bool EconomyCanAfford(const Economy *economy, ResourceType type, float amount);
bool EconomySpend(Economy *economy, ResourceType type, float amount);
const char *EconomyResourceName(ResourceType type);

#endif
