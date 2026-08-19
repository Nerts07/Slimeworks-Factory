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

static void TestUpgradeTargetsNearbyStation(void)
{
    Economy economy = RichEconomy();
    economy.amount[RESOURCE_SLIME] = 20.0f;
    BuildingSystem buildings;
    BuildingsInit(&buildings);
    BuildingsTryPlace(&buildings, &economy, BUILD_REFINERY, (Vector3){ 5, 0, 5 }, RESOURCE_COUNT);
    Check("upgrade reports no station outside interaction range",
          BuildingsTryUpgrade(&buildings, &economy, (Vector3){ 0, 0, 0 }, 2.4f) == BUILDING_UPGRADE_NO_STATION);
    Check("nearby station upgrades successfully",
          BuildingsTryUpgrade(&buildings, &economy, (Vector3){ 5, 0, 6.5f }, 2.4f) == BUILDING_UPGRADE_OK);
    Check("upgrade raises station to level two", buildings.items[0].level == 2);
    Check("second upgrade reports maximum level",
          BuildingsTryUpgrade(&buildings, &economy, (Vector3){ 5, 0, 6.5f }, 2.4f) == BUILDING_UPGRADE_MAX_LEVEL);
}

static void TestPipeFlowDirectionFollowsRoute(void)
{
    Economy economy = RichEconomy();
    BuildingSystem buildings;
    BuildingsInit(&buildings);
    BuildingsTryPlace(&buildings, &economy, BUILD_REFINERY, (Vector3){ 0, 0, 0 }, RESOURCE_COUNT);
    BuildingsTryPlace(&buildings, &economy, BUILD_PIPE, (Vector3){ 1, 0, 0 }, RESOURCE_COUNT);
    BuildingsTryPlace(&buildings, &economy, BUILD_PIPE, (Vector3){ 2, 0, 0 }, RESOURCE_COUNT);
    BuildingsTryPlace(&buildings, &economy, BUILD_PUMP, (Vector3){ 3, 0, 0 }, RESOURCE_GREEN_ORE);
    RunFor(&buildings, &economy, 1.0f);
    Check("pipe particle points from pump toward left receiver", buildings.items[1].flowDirection.x < -0.9f);
    Check("every segment receives its route direction", buildings.items[2].flowDirection.x < -0.9f);

    Economy verticalEconomy = RichEconomy();
    BuildingSystem vertical;
    BuildingsInit(&vertical);
    BuildingsTryPlace(&vertical, &verticalEconomy, BUILD_PUMP, (Vector3){ 0, 0, 0 }, RESOURCE_GREEN_ORE);
    BuildingsTryPlace(&vertical, &verticalEconomy, BUILD_PIPE, (Vector3){ 0, 0, 1 }, RESOURCE_COUNT);
    BuildingsTryPlace(&vertical, &verticalEconomy, BUILD_REFINERY, (Vector3){ 0, 0, 2 }, RESOURCE_COUNT);
    RunFor(&vertical, &verticalEconomy, 1.0f);
    Check("pipe particle follows Z-axis route", vertical.items[1].flowDirection.y > 0.9f);
}

int main(void)
{
    TestStraightNetwork();
    TestLateConnectionRecoversFullPump();
    TestRoutingUsesMultipleReceivers();
    TestUpgradeTargetsNearbyStation();
    TestPipeFlowDirectionFollowsRoute();
    if (failures) printf("\n%d building-network test(s) failed.\n", failures);
    else printf("\nAll building-network tests passed.\n");
    return failures != 0;
}
