#include "save.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

#define SAVE_MAGIC 0x534C4D31u
#define SAVE_VERSION 4u
#define SAVE_PATH "saves/slimeworks.save"

typedef struct SaveFile {
    uint32_t magic;
    uint32_t version;
    GameState state;
} SaveFile;

bool SaveGameWrite(const GameState *state)
{
    (void)mkdir("saves", 0755);
    FILE *file = fopen(SAVE_PATH, "wb");
    if (!file) return false;
    SaveFile data = { SAVE_MAGIC, SAVE_VERSION, *state };
    bool ok = fwrite(&data, sizeof(data), 1, file) == 1;
    fclose(file);
    return ok;
}

bool SaveGameRead(GameState *state)
{
    FILE *file = fopen(SAVE_PATH, "rb");
    if (!file) return false;
    SaveFile data = { 0 };
    bool ok = fread(&data, sizeof(data), 1, file) == 1 && data.magic == SAVE_MAGIC && data.version == SAVE_VERSION;
    fclose(file);
    if (ok && (data.state.buildings.count < 0 || data.state.buildings.count > BUILDING_MAX ||
               data.state.nodes.count < 0 || data.state.nodes.count > RESOURCE_NODE_MAX ||
               data.state.contracts.step < TUTORIAL_HARVEST_GREEN || data.state.contracts.step > TUTORIAL_COMPLETE)) ok = false;
    if (ok) *state = data.state;
    return ok;
}
