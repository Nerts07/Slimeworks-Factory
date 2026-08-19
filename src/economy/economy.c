#include "economy.h"

void EconomyInit(Economy *economy)
{
    *economy = (Economy){ 0 };
}

void EconomyAdd(Economy *economy, ResourceType type, float amount)
{
    if (type >= 0 && type < RESOURCE_COUNT) economy->amount[type] += amount;
}

bool EconomyCanAfford(const Economy *economy, ResourceType type, float amount)
{
    return type >= 0 && type < RESOURCE_COUNT && economy->amount[type] + 0.0001f >= amount;
}

bool EconomySpend(Economy *economy, ResourceType type, float amount)
{
    if (!EconomyCanAfford(economy, type, amount)) return false;
    economy->amount[type] -= amount;
    return true;
}

const char *EconomyResourceName(ResourceType type)
{
    static const char *names[RESOURCE_COUNT] = { "Green ore", "Acid ore", "Crystal ore", "Slime" };
    return (type >= 0 && type < RESOURCE_COUNT) ? names[type] : "Unknown";
}
