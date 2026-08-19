#include "world_selector.h"

#include <raylib.h>
#include <stdio.h>
#include <string.h>

static unsigned int NextSeed(WorldSelector *selector)
{
    selector->seedNonce += 0x9e3779b9u;
    unsigned int seed = selector->seedNonce ^ (unsigned int)(GetTime() * 1000000.0);
    seed ^= seed >> 16;
    seed *= 0x7feb352du;
    seed ^= seed >> 15;
    return seed ? seed : 1u;
}

static int ChoiceCount(const WorldSelector *selector)
{
    return 1 + selector->catalog->authoredCount;
}

void WorldSelectorInit(WorldSelector *selector, LevelCatalog *catalog)
{
    *selector = (WorldSelector){ .catalog = catalog, .seedNonce = 0x51c3e8a1u };
    selector->generatedSeed = NextSeed(selector);
}

void WorldSelectorUpdate(WorldSelector *selector)
{
    int count = ChoiceCount(selector);
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        selector->selectedIndex = (selector->selectedIndex + count - 1) % count;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        selector->selectedIndex = (selector->selectedIndex + 1) % count;
    if (IsKeyPressed(KEY_R)) {
        LevelCatalogRefresh(selector->catalog);
        selector->selectedIndex = 0;
        selector->generatedSeed = NextSeed(selector);
    }
    if (IsKeyPressed(KEY_ESCAPE)) { selector->cancelled = true; return; }
    if (!IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_SPACE)) return;

    if (selector->selectedIndex == 0) {
        selector->choice = (WorldChoice){ .type = WORLD_CHOICE_GENERATED, .seed = selector->generatedSeed };
    } else {
        const AuthoredLevelEntry *entry = &selector->catalog->authored[selector->selectedIndex - 1];
        selector->choice = (WorldChoice){ .type = WORLD_CHOICE_AUTHORED };
        snprintf(selector->choice.levelPath, sizeof(selector->choice.levelPath), "%s", entry->path);
    }
    selector->confirmed = true;
}

static void DrawCard(int x, int y, int width, int height, bool selected, Color accent)
{
    Color fill = selected ? Fade(accent, 0.30f) : (Color){ 15, 25, 38, 225 };
    DrawRectangleRounded((Rectangle){ (float)x, (float)y, (float)width, (float)height }, 0.12f, 8, fill);
    DrawRectangleRoundedLines((Rectangle){ (float)x, (float)y, (float)width, (float)height }, 0.12f, 8, selected ? accent : Fade(WHITE, 0.16f));
}

void WorldSelectorDraw(const WorldSelector *selector)
{
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    BeginDrawing();
    ClearBackground((Color){ 11, 19, 33, 255 });
    for (int i = 0; i < 12; ++i) {
        float x = (float)((i * 197) % sw);
        float y = (float)((i * 83) % sh);
        DrawCircleV((Vector2){ x, y }, 100.0f + (float)(i % 3) * 45.0f, Fade((i & 1) ? VIOLET : SKYBLUE, 0.045f));
    }
    DrawText("SLIMEWORKS FACTORY", 56, 44, 34, RAYWHITE);
    DrawText("WORLD LAUNCHER", 58, 84, 18, SKYBLUE);
    DrawText("Choose an authored .levl world or generate a fresh expedition.", 58, 118, 18, Fade(RAYWHITE, 0.75f));

    const int cardX = 58, cardWidth = sw - 116, cardHeight = 84;
    int y = 170;
    bool generatedSelected = selector->selectedIndex == 0;
    DrawCard(cardX, y, cardWidth, cardHeight, generatedSelected, LIME);
    DrawText("GENERATED EXPEDITION", cardX + 24, y + 17, 22, generatedSelected ? LIME : RAYWHITE);
    char seedLine[96];
    snprintf(seedLine, sizeof(seedLine), "Seed %06X — randomized routes, elevations and palette", selector->generatedSeed & 0x00ffffffu);
    DrawText(seedLine, cardX + 24, y + 48, 16, Fade(RAYWHITE, 0.76f));
    y += cardHeight + 26;

    DrawText("AUTHORED WORLDS  —  levels/*.levl", cardX, y, 18, GOLD);
    y += 28;
    if (selector->catalog->authoredCount == 0) {
        DrawText("No .levl files found. Export from Slimeworks Level Editor into the levels folder, then press R.", cardX, y + 10, 16, GRAY);
    }
    for (int i = 0; i < selector->catalog->authoredCount; ++i) {
        bool selected = selector->selectedIndex == i + 1;
        DrawCard(cardX, y, cardWidth, 58, selected, GOLD);
        DrawText(selector->catalog->authored[i].title, cardX + 20, y + 10, 20, selected ? GOLD : RAYWHITE);
        DrawText(selector->catalog->authored[i].path, cardX + 20, y + 34, 14, Fade(RAYWHITE, 0.52f));
        y += 68;
        if (y > sh - 112) break;
    }

    DrawRectangle(0, sh - 78, sw, 78, Fade(BLACK, 0.35f));
    DrawText("UP/DOWN or W/S: choose    ENTER / SPACE: launch    R: refresh worlds + new seed    ESC: quit", 58, sh - 49, 16, LIGHTGRAY);
    EndDrawing();
}
