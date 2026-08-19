#ifndef SLIMEWORKS_CONTRACTS_H
#define SLIMEWORKS_CONTRACTS_H

#include "../build/building.h"

typedef enum TutorialStep {
    TUTORIAL_HARVEST_GREEN,
    TUTORIAL_PLACE_PUMP,
    TUTORIAL_PLACE_PIPES,
    TUTORIAL_PLACE_REFINERY,
    TUTORIAL_DELIVER_SLIME,
    TUTORIAL_UNLOCK_ACID,
    TUTORIAL_PLACE_MIXER,
    TUTORIAL_COMPLETE
} TutorialStep;

typedef struct ContractSystem {
    TutorialStep step;
    int completedContracts;
    bool mixerUnlocked;
} ContractSystem;

void ContractsInit(ContractSystem *contracts);
bool ContractsUpdate(ContractSystem *contracts, Economy *economy, const BuildingSystem *buildings, bool zoneTwoUnlocked);
const char *ContractsObjective(const ContractSystem *contracts);
const char *ContractsDetail(const ContractSystem *contracts, const Economy *economy, const BuildingSystem *buildings);
bool ContractsAllowsBuild(const ContractSystem *contracts, BuildType type);

#endif
