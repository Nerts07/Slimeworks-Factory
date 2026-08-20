#include "save.h"

#include <math.h>
#include <raymath.h>
#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#define SAVE_MAGIC 0x534C4D31u
#define SAVE_VERSION 6u
#define SAVE_PATH "saves/slimeworks.save"

typedef struct SavedPlayer {
    Vector3 position;
    float yaw;
    float pitch;
} SavedPlayer;

typedef struct SaveFile {
    uint32_t magic;
    uint32_t version;
    GameState state;
    SavedPlayer player;
} SaveFile;

/* Version 4 stored Building before directional pipe flow and did not store Player. */
typedef struct LegacyBuildingV4 {
    BuildType type;
    Vector3 position;
    int level;
    ResourceType sourceResource;
    float buffer[RESOURCE_COUNT];
    float progress;
    float flowTimer;
    ResourceType flowResource;
    BuildingStatus status;
} LegacyBuildingV4;

typedef struct LegacyBuildingSystemV4 {
    LegacyBuildingV4 items[BUILDING_MAX];
    int count;
} LegacyBuildingSystemV4;

typedef struct LegacyGameStateV4 {
    Economy economy;
    ResourceNodeSystem nodes;
    LegacyBuildingSystemV4 buildings;
    ContractSystem contracts;
    bool zoneTwoUnlocked;
    float sessionSeconds;
    int rareSlimesCollected;
    Vector3 gatePosition;
    Vector3 rareSlimeSpawn;
    RareSlime rareSlime;
} LegacyGameStateV4;

typedef struct LegacySaveFileV4 {
    uint32_t magic;
    uint32_t version;
    LegacyGameStateV4 state;
} LegacySaveFileV4;

static bool ValidateState(const GameState *state)
{
    return state->buildings.count >= 0 && state->buildings.count <= BUILDING_MAX &&
           state->nodes.count >= 0 && state->nodes.count <= RESOURCE_NODE_MAX &&
           state->contracts.step >= TUTORIAL_HARVEST_GREEN && state->contracts.step <= TUTORIAL_COMPLETE;
}

static bool ValidatePlayer(const SavedPlayer *player)
{
    return isfinite(player->position.x) && isfinite(player->position.y) && isfinite(player->position.z) &&
           isfinite(player->yaw) && isfinite(player->pitch) &&
           fabsf(player->position.x) < 10000.0f && fabsf(player->position.y) < 10000.0f &&
           fabsf(player->position.z) < 10000.0f;
}

static void RestorePlayer(Player *player, const SavedPlayer *saved, const Level *level)
{
    player->position = saved->position;
    player->yaw = saved->yaw;
    player->pitch = Clamp(saved->pitch, -1.45f, 1.45f);
    player->velocity = (Vector2){ 0 };
    player->verticalVelocity = 0.0f;
    float ground = level ? LevelGroundHeight(level, player->position.x, player->position.z) : player->position.y;
    if (player->position.y < ground) player->position.y = ground;
    player->grounded = fabsf(player->position.y - ground) <= 0.05f;
}

static void ConvertLegacyState(GameState *destination, const LegacyGameStateV4 *legacy)
{
    GameState converted = *destination;
    converted.economy = legacy->economy;
    converted.nodes = legacy->nodes;
    converted.contracts = legacy->contracts;
    converted.zoneTwoUnlocked = legacy->zoneTwoUnlocked;
    converted.sessionSeconds = legacy->sessionSeconds;
    converted.rareSlimesCollected = legacy->rareSlimesCollected;
    converted.gatePosition = legacy->gatePosition;
    converted.rareSlimeSpawn = legacy->rareSlimeSpawn;
    converted.rareSlime = legacy->rareSlime;
    converted.buildings = (BuildingSystem){ .count = legacy->buildings.count };
    for (int i = 0; i < legacy->buildings.count && i < BUILDING_MAX; ++i) {
        const LegacyBuildingV4 *source = &legacy->buildings.items[i];
        Building *target = &converted.buildings.items[i];
        target->type = source->type;
        target->position = source->position;
        target->level = source->level;
        target->sourceResource = source->sourceResource;
        for (int resource = 0; resource < RESOURCE_COUNT; ++resource) target->buffer[resource] = source->buffer[resource];
        target->progress = source->progress;
        target->flowTimer = 0.0f;
        target->flowResource = source->flowResource;
        target->status = source->status;
    }
    *destination = converted;
}

bool SaveGameWrite(const GameState *state, const Player *player)
{
    if (!state || !player) return false;
#ifdef _WIN32
    (void)_mkdir("saves");
#else
    (void)mkdir("saves", 0755);
#endif
    FILE *file = fopen(SAVE_PATH, "wb");
    if (!file) return false;
    SaveFile data = {
        .magic = SAVE_MAGIC,
        .version = SAVE_VERSION,
        .state = *state,
        .player = { .position = player->position, .yaw = player->yaw, .pitch = player->pitch }
    };
    bool ok = fwrite(&data, sizeof(data), 1, file) == 1;
    fclose(file);
    return ok;
}

bool SaveGameRead(GameState *state, Player *player, const Level *level)
{
    if (!state || !player) return false;
    FILE *file = fopen(SAVE_PATH, "rb");
    if (!file) return false;
    uint32_t header[2] = { 0 };
    bool headerOk = fread(header, sizeof(header), 1, file) == 1 && header[0] == SAVE_MAGIC;
    rewind(file);
    bool ok = false;
    if (headerOk && header[1] == SAVE_VERSION) {
        SaveFile data = { 0 };
        ok = fread(&data, sizeof(data), 1, file) == 1 && ValidateState(&data.state) && ValidatePlayer(&data.player);
        if (ok) {
            *state = data.state;
            RestorePlayer(player, &data.player, level);
        }
    } else if (headerOk && header[1] == 4u) {
        LegacySaveFileV4 legacy = { 0 };
        ok = fread(&legacy, sizeof(legacy), 1, file) == 1 &&
             legacy.state.buildings.count >= 0 && legacy.state.buildings.count <= BUILDING_MAX;
        if (ok) {
            ConvertLegacyState(state, &legacy.state);
            ok = ValidateState(state);
        }
    }
    fclose(file);
    return ok;
}
