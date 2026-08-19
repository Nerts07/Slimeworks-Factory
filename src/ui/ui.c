#include "ui.h"
#include <stdio.h>
#include <math.h>

static void DrawPanel(int x, int y, int width, int height)
{
    DrawRectangle(x, y, width, height, (Color){ 12, 20, 30, 215 });
    DrawRectangleLines(x, y, width, height, Fade(SKYBLUE, 0.65f));
}

static const char *DistrictName(Vector3 position)
{
    if (position.x < -22.0f) return "SLIME RANCH";
    if (position.x > 24.0f && position.z < -24.0f) return "CRYSTAL DECK";
    if (position.x > 24.0f && position.z > 24.0f) return "ORANGE TEST RANGE";
    if (position.x > 24.0f) return "ACID WORKS";
    if (position.z > 24.0f) return "SKY LAB MARKET";
    if (fabsf(position.x) <= 16.0f && fabsf(position.z) <= 16.0f) return "CENTRAL SLIMEWORKS";
    return "LOWER SERVICE YARD";
}

void UiDrawHud(const Player *player, const GameState *state, bool mouseCaptured, bool buildMode,
               BuildType selectedBuild, const ResourceNode *targetNode, const Building *nearbyBuilding,
               float harvestProgress, const char *notification, float notificationSeconds, bool helpOpen)
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawCircle(sw / 2, sh / 2, 3, RAYWHITE);
    const char *district = DistrictName(player->position);
    int districtWidth = MeasureText(district, 17);
    DrawText(district, sw / 2 - districtWidth / 2, 18, 17, Fade(WHITE, 0.85f));
    DrawPanel(16, 16, 244, 118);
    DrawText("SLIMEWORKS // MVP", 29, 28, 19, RAYWHITE);
    char line[96];
    snprintf(line, sizeof(line), "Green ore: %.0f", state->economy.amount[RESOURCE_GREEN_ORE]); DrawText(line, 29, 54, 16, LIME);
    snprintf(line, sizeof(line), "Acid ore: %.0f  Crystal: %.0f", state->economy.amount[RESOURCE_ACID_ORE], state->economy.amount[RESOURCE_CRYSTAL_ORE]); DrawText(line, 29, 74, 14, YELLOW);
    snprintf(line, sizeof(line), "Slime: %.1f", state->economy.amount[RESOURCE_SLIME]); DrawText(line, 29, 96, 18, SKYBLUE);

    DrawPanel(16, sh - 72, 570, 56);
    DrawText("B build   E interact/place   R remove pipe   U upgrade   F1 help", 29, sh - 56, 14, LIGHTGRAY);
    DrawText(mouseCaptured ? "ESC unlock mouse   F5 save   F9 load" : "ESC capture mouse", 29, sh - 35, 12, GRAY);

    DrawPanel(sw / 2 - 235, 44, 470, 58);
    DrawText("NEXT: ", sw / 2 - 220, 55, 17, GOLD);
    DrawText(ContractsObjective(&state->contracts), sw / 2 - 162, 55, 17, RAYWHITE);
    DrawText(ContractsDetail(&state->contracts, &state->economy, &state->buildings), sw / 2 - 220, 78, 14, LIGHTGRAY);

    if (buildMode) {
        DrawPanel(sw - 310, 16, 294, 150);
        snprintf(line, sizeof(line), "BUILD: %s (%.0f green ore)", BuildingName(selectedBuild), BuildingCost(selectedBuild));
        DrawText(line, sw - 294, 30, 17, ContractsAllowsBuild(&state->contracts, selectedBuild) ? GOLD : RED);
        DrawText("1 Pump: place beside ore node", sw - 294, 56, 14, LIGHTGRAY);
        DrawText("2 Refinery: green ore -> slime", sw - 294, 76, 14, LIGHTGRAY);
        DrawText("3 Acid Mixer: locked until Zone 2", sw - 294, 96, 14, LIGHTGRAY);
        DrawText("4 Booster: nearby station speed", sw - 294, 116, 14, LIGHTGRAY);
        DrawText("5 Pipe: connect stations in a line", sw - 294, 136, 14, LIGHTGRAY);
    } else if (targetNode) {
        DrawText("Hold E to harvest", sw / 2 - 73, sh / 2 + 25, 16, RAYWHITE);
        DrawRectangle(sw / 2 - 80, sh / 2 + 48, 160, 10, Fade(BLACK, 0.65f));
        DrawRectangle(sw / 2 - 80, sh / 2 + 48, (int)(160 * harvestProgress), 10, LIME);
    }

    if (!state->zoneTwoUnlocked) {
        snprintf(line, sizeof(line), "Zone 2 locked: %.0f / 55 slime. Press O near the gate.", state->economy.amount[RESOURCE_SLIME]);
        DrawText(line, 290, 22, 16, ORANGE);
    }
    if (nearbyBuilding && !buildMode) {
        int panelX = sw - 356, panelY = sh - 160;
        DrawPanel(panelX, panelY, 340, 144);
        snprintf(line, sizeof(line), "%s L%d", BuildingName(nearbyBuilding->type), nearbyBuilding->level);
        DrawText(line, panelX + 16, panelY + 14, 18, RAYWHITE);
        DrawText(BuildingStatusText(nearbyBuilding->status), panelX + 16, panelY + 39, 15,
                 nearbyBuilding->status == BUILDING_WORKING ? LIME : ORANGE);
        if (nearbyBuilding->type == BUILD_PUMP) {
            ResourceType source = nearbyBuilding->sourceResource;
            snprintf(line, sizeof(line), "Source: %s   Stored: %.1f / %.0f", EconomyResourceName(source),
                     nearbyBuilding->buffer[source], BUILDING_BUFFER_CAPACITY);
            DrawText(line, panelX + 16, panelY + 65, 14, LIGHTGRAY);
            DrawText("Required path: Pump -> Pipes -> Processor", panelX + 16, panelY + 91, 14, SKYBLUE);
        } else if (nearbyBuilding->type == BUILD_REFINERY) {
            snprintf(line, sizeof(line), "Green input: %.1f / %.0f", nearbyBuilding->buffer[RESOURCE_GREEN_ORE], BUILDING_BUFFER_CAPACITY);
            DrawText(line, panelX + 16, panelY + 65, 14, LIGHTGRAY);
            DrawText("Turns green ore into wallet Slime", panelX + 16, panelY + 91, 14, SKYBLUE);
        } else if (nearbyBuilding->type == BUILD_MIXER) {
            snprintf(line, sizeof(line), "Green: %.1f   Acid: %.1f", nearbyBuilding->buffer[RESOURCE_GREEN_ORE], nearbyBuilding->buffer[RESOURCE_ACID_ORE]);
            DrawText(line, panelX + 16, panelY + 65, 14, LIGHTGRAY);
            DrawText("Needs both resources through pipes", panelX + 16, panelY + 91, 14, SKYBLUE);
        } else if (nearbyBuilding->type == BUILD_PIPE) {
            DrawText("Blue links = connected network", panelX + 16, panelY + 65, 14, LIGHTGRAY);
            DrawText("Moving colored dot = active transfer", panelX + 16, panelY + 91, 14, SKYBLUE);
        } else {
            DrawText("Boosts processors within 3 tiles", panelX + 16, panelY + 65, 14, GOLD);
            DrawText("No pipe connection required", panelX + 16, panelY + 91, 14, LIGHTGRAY);
        }
        DrawText("Red light above a station = needs attention", panelX + 16, panelY + 117, 12, GRAY);
    }
    if (helpOpen) {
        DrawPanel(sw / 2 - 260, sh / 2 - 150, 520, 300);
        DrawText("HOW THE FACTORY WORKS", sw / 2 - 210, sh / 2 - 128, 22, GOLD);
        DrawText("1. Harvest green ore with E.", sw / 2 - 210, sh / 2 - 92, 17, RAYWHITE);
        DrawText("2. Put a Pump beside a node, then place Pipes.", sw / 2 - 210, sh / 2 - 62, 17, RAYWHITE);
        DrawText("3. Connect Pipes to a Refinery to make Slime.", sw / 2 - 210, sh / 2 - 32, 17, RAYWHITE);
        DrawText("4. Follow NEXT at the top. It unlocks the next tool.", sw / 2 - 210, sh / 2 - 2, 17, RAYWHITE);
        DrawText("Pipes only connect on side-by-side grid tiles.", sw / 2 - 210, sh / 2 + 38, 16, SKYBLUE);
        DrawText("Press F1 to close", sw / 2 - 210, sh / 2 + 82, 15, GRAY);
    }
    if (notificationSeconds > 0.0f) {
        int width = MeasureText(notification, 20) + 32;
        DrawPanel(sw / 2 - width / 2, 92, width, 38);
        DrawText(notification, sw / 2 - width / 2 + 16, 102, 20, GOLD);
    }
}
