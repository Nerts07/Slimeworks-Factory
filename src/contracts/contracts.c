#include "contracts.h"
#include <stdio.h>

void ContractsInit(ContractSystem *contracts)
{
    *contracts = (ContractSystem){ .step = TUTORIAL_HARVEST_GREEN };
}

bool ContractsUpdate(ContractSystem *contracts, Economy *economy, const BuildingSystem *buildings, bool zoneTwoUnlocked)
{
    TutorialStep before = contracts->step;
    switch (contracts->step) {
        case TUTORIAL_HARVEST_GREEN:
            if (economy->amount[RESOURCE_GREEN_ORE] >= 8.0f) {
                EconomyAdd(economy, RESOURCE_GREEN_ORE, 12.0f);
                contracts->step++;
            }
            break;
        case TUTORIAL_PLACE_PUMP:
            if (BuildingsCountType(buildings, BUILD_PUMP) >= 1) contracts->step++;
            break;
        case TUTORIAL_PLACE_PIPES:
            if (BuildingsCountType(buildings, BUILD_PIPE) >= 2) contracts->step++;
            break;
        case TUTORIAL_PLACE_REFINERY:
            if (BuildingsCountType(buildings, BUILD_REFINERY) >= 1) contracts->step++;
            break;
        case TUTORIAL_DELIVER_SLIME:
            if (economy->amount[RESOURCE_SLIME] >= 20.0f) {
                EconomySpend(economy, RESOURCE_SLIME, 20.0f);
                EconomyAdd(economy, RESOURCE_GREEN_ORE, 20.0f);
                contracts->completedContracts++;
                contracts->step++;
            }
            break;
        case TUTORIAL_UNLOCK_ACID:
            if (zoneTwoUnlocked) {
                EconomyAdd(economy, RESOURCE_GREEN_ORE, 12.0f);
                contracts->mixerUnlocked = true;
                contracts->step++;
            }
            break;
        case TUTORIAL_PLACE_MIXER:
            if (BuildingsCountType(buildings, BUILD_MIXER) >= 1 && BuildingsHasPumpFor(buildings, RESOURCE_ACID_ORE)) {
                contracts->completedContracts++;
                contracts->step++;
            }
            break;
        case TUTORIAL_COMPLETE:
            break;
    }
    return before != contracts->step;
}

const char *ContractsObjective(const ContractSystem *contracts)
{
    static const char *objectives[] = {
        "Collect 8 green ore",
        "Build a pump beside a green ore node",
        "Lay 2 pipe segments from the pump",
        "Build a refinery next to the pipe",
        "Contract: deliver 20 slime",
        "Open the Acid Works gate for 55 slime",
        "Build an Acid Mixer and connect acid + green pumps",
        "Factory tutorial complete — expand your network"
    };
    return objectives[contracts->step];
}

const char *ContractsDetail(const ContractSystem *contracts, const Economy *economy, const BuildingSystem *buildings)
{
    static char detail[128];
    switch (contracts->step) {
        case TUTORIAL_HARVEST_GREEN: snprintf(detail, sizeof(detail), "%.0f / 8 green ore — hold E on glowing green nodes", economy->amount[RESOURCE_GREEN_ORE]); break;
        case TUTORIAL_PLACE_PUMP: snprintf(detail, sizeof(detail), "Press B, choose 1, place next to a green node"); break;
        case TUTORIAL_PLACE_PIPES: snprintf(detail, sizeof(detail), "%d / 2 pipes — choose 5 in build mode", BuildingsCountType(buildings, BUILD_PIPE)); break;
        case TUTORIAL_PLACE_REFINERY: snprintf(detail, sizeof(detail), "Choose 2 and place it touching the pipe"); break;
        case TUTORIAL_DELIVER_SLIME: snprintf(detail, sizeof(detail), "%.0f / 20 slime — refinery must be connected", economy->amount[RESOURCE_SLIME]); break;
        case TUTORIAL_UNLOCK_ACID: snprintf(detail, sizeof(detail), "Go east to the neon gate and press O"); break;
        case TUTORIAL_PLACE_MIXER: snprintf(detail, sizeof(detail), "Build Mixer (3) and Acid Pump (1), then connect pipes"); break;
        default: snprintf(detail, sizeof(detail), "Try contracts, upgrades and the Crystal Deck"); break;
    }
    return detail;
}

bool ContractsAllowsBuild(const ContractSystem *contracts, BuildType type)
{
    if (type == BUILD_MIXER) return contracts->mixerUnlocked;
    return type >= BUILD_PUMP && type < BUILD_COUNT;
}
