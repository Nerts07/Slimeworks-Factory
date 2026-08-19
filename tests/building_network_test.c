#include "build/building.h"

#include <stdio.h>

static int failures;

static void Check(const char *name, bool condition)
{
    printf("%-48s %s\n", name, condition ? "PASS" : "FAIL");
    if (!condition) failures++;
}

static void RunFor(BuildingSystem *buildings, Economy *economy, float seconds)
{
    for (int frame = 0; frame < (int)(seconds * 60.0f); ++frame)
        BuildingsUpdate(buildings, economy, 1.0f / 60.0f);
}

static Economy RichEconomy(void)
{
    Economy economy = { 0 };
    economy.amount[RESOURCE_GREEN_ORE] = 1000.0f;
    return economy;
}

static void TestStraightNetwork(void)
{
    Economy economy = RichEconomy();
    BuildingSystem buildings;
    BuildingsInit(&buildings);
    BuildingsTryPlace(&buildings, &economy, BUILD_PUMP, (Vector3){ 0, 0, 0 }, RESOURCE_GREEN_ORE);
    BuildingsTryPlace(&buildings, &economy, BUILD_PIPE, (Vector3){ 1, 0, 0 }, RESOURCE_COUNT);
    BuildingsTryPlace(&buildings, &economy, BUILD_PIPE, (Vector3){ 2, 0, 0 }, RESOURCE_COUNT);
    BuildingsTryPlace(&buildings, &economy, BUILD_REFINERY, (Vector3){ 3, 0, 0 }, RESOURCE_COUNT);
    RunFor(&buildings, &economy, 4.0f);
    Check("straight pump -> pipes -> refinery makes slime", economy.amount[RESOURCE_SLIME] > 1.0f);
    Check("pump reports working while transferring", buildings.items[0].status == BUILDING_WORKING);
}

static void TestLateConnectionRecoversFullPump(void)
{
    Economy economy = RichEconomy();
    BuildingSystem buildings;
    BuildingsInit(&buildings);
    BuildingsTryPlace(&buildings, &economy, BUILD_PUMP, (Vector3){ 0, 0, 0 }, RESOURCE_GREEN_ORE);
    BuildingsTryPlace(&buildings, &economy, BUILD_PIPE, (Vector3){ 1, 0, 0 }, RESOURCE_COUNT);
    RunFor(&buildings, &economy, 10.0f);
    Check("unconnected pump fills its internal buffer", buildings.items[0].buffer[RESOURCE_GREEN_ORE] >= BUILDING_BUFFER_CAPACITY - 0.01f);
    BuildingsTryPlace(&buildings, &economy, BUILD_PIPE, (Vector3){ 2, 0, 0 }, RESOURCE_COUNT);
    BuildingsTryPlace(&buildings, &economy, BUILD_REFINERY, (Vector3){ 3, 0, 0 }, RESOURCE_COUNT);
    RunFor(&buildings, &economy, 3.0f);
    Check("full pump resumes after receiver is connected", economy.amount[RESOURCE_SLIME] > 1.0f);
}

static void TestRoutingUsesMultipleReceivers(void)
{
    Economy economy = RichEconomy();
    BuildingSystem buildings;
    BuildingsInit(&buildings);
    BuildingsTryPlace(&buildings, &economy, BUILD_PUMP, (Vector3){ 0, 0, 0 }, RESOURCE_GREEN_ORE);
    BuildingsTryPlace(&buildings, &economy, BUILD_PIPE, (Vector3){ 1, 0, 0 }, RESOURCE_COUNT);
    BuildingsTryPlace(&buildings, &economy, BUILD_PIPE, (Vector3){ 2, 0, 0 }, RESOURCE_COUNT);
    BuildingsTryPlace(&buildings, &economy, BUILD_REFINERY, (Vector3){ 3, 0, 0 }, RESOURCE_COUNT);
    BuildingsTryPlace(&buildings, &economy, BUILD_REFINERY, (Vector3){ 2, 0, 1 }, RESOURCE_COUNT);
    RunFor(&buildings, &economy, 4.0f);
    Check("first receiver receives and processes ore", buildings.items[3].progress > 0.1f);
    Check("second receiver is not starved by routing", buildings.items[4].progress > 0.1f);
}

int main(void)
{
    TestStraightNetwork();
    TestLateConnectionRecoversFullPump();
    TestRoutingUsesMultipleReceivers();
    if (failures) printf("\n%d building-network test(s) failed.\n", failures);
    else printf("\nAll building-network tests passed.\n");
    return failures != 0;
}
