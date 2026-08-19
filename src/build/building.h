#ifndef SLIMEWORKS_BUILDING_H
#define SLIMEWORKS_BUILDING_H

#include <raylib.h>
#include "../economy/economy.h"
#include "../render/game_visuals.h"

#define BUILDING_MAX 64
#define BUILDING_BUFFER_CAPACITY 12.0f

typedef enum BuildType {
    BUILD_PUMP,
    BUILD_REFINERY,
    BUILD_MIXER,
    BUILD_BOOSTER,
    BUILD_PIPE,
    BUILD_COUNT
} BuildType;

typedef enum BuildingStatus {
    BUILDING_WORKING,
    BUILDING_NEEDS_PIPE,
    BUILDING_NEEDS_DESTINATION,
    BUILDING_NEEDS_INPUT,
    BUILDING_OUTPUT_FULL,
    BUILDING_NEEDS_SOURCE
} BuildingStatus;

typedef struct Building {
    BuildType type;
    Vector3 position;
    int level;
    ResourceType sourceResource;
    float buffer[RESOURCE_COUNT];
    float progress;
    float flowTimer;
    ResourceType flowResource;
    Vector2 flowDirection;
    BuildingStatus status;
} Building;

typedef struct BuildingSystem {
    Building items[BUILDING_MAX];
    int count;
} BuildingSystem;

typedef enum BuildingUpgradeResult {
    BUILDING_UPGRADE_OK,
    BUILDING_UPGRADE_NO_STATION,
    BUILDING_UPGRADE_MAX_LEVEL,
    BUILDING_UPGRADE_NOT_ENOUGH_SLIME
} BuildingUpgradeResult;

void BuildingsInit(BuildingSystem *system);
void BuildingsUpdate(BuildingSystem *system, Economy *economy, float dt);
bool BuildingsTryPlace(BuildingSystem *system, Economy *economy, BuildType type, Vector3 position, ResourceType sourceResource);
bool BuildingsTryRemovePipe(BuildingSystem *system, Economy *economy, Vector3 position);
BuildingUpgradeResult BuildingsTryUpgrade(BuildingSystem *system, Economy *economy, Vector3 position, float maxDistance);
const char *BuildingName(BuildType type);
const char *BuildingStatusText(BuildingStatus status);
float BuildingCost(BuildType type);
int BuildingsCountType(const BuildingSystem *system, BuildType type);
bool BuildingsHasPumpFor(const BuildingSystem *system, ResourceType resource);
const Building *BuildingsFindNearest(const BuildingSystem *system, Vector3 position, float maxDistance);
void BuildingsDraw(const BuildingSystem *system, const GameVisuals *visuals, bool buildMode, BuildType selected,
                   Vector3 previewPosition, bool previewValid);

#endif
