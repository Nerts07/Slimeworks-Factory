#ifndef SLIMEWORKS_WORLD_SELECTOR_H
#define SLIMEWORKS_WORLD_SELECTOR_H

#include "../world/level_catalog.h"

typedef enum WorldChoiceType {
    WORLD_CHOICE_GENERATED,
    WORLD_CHOICE_AUTHORED
} WorldChoiceType;

typedef struct WorldChoice {
    WorldChoiceType type;
    unsigned int seed;
    char levelPath[LEVEL_CATALOG_PATH_LENGTH];
} WorldChoice;

typedef struct WorldSelector {
    LevelCatalog *catalog;
    int selectedIndex;
    unsigned int generatedSeed;
    unsigned int seedNonce;
    bool confirmed;
    bool cancelled;
    WorldChoice choice;
} WorldSelector;

void WorldSelectorInit(WorldSelector *selector, LevelCatalog *catalog);
void WorldSelectorUpdate(WorldSelector *selector);
void WorldSelectorDraw(const WorldSelector *selector);

#endif
