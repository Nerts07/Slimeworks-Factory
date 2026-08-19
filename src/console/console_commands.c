#include "console.h"
#include "../save/save.h"
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool CommandHelp(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize);
static bool CommandGive(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize);
static bool CommandTeleport(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize);
static bool CommandSave(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize);
static bool CommandLoad(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize);
static bool CommandUnlockZone(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize);
static bool CommandContract(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize);
static bool CommandClear(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize);

#define CONSOLE_COMMAND(id, description, usage, parameters, minArgs, maxArgs, handler) \
    { #id, description, usage, parameters, minArgs, maxArgs, handler },
static const ConsoleCommand COMMANDS[] = {
#include "console_commands.def"
};
#undef CONSOLE_COMMAND

const ConsoleCommand *DeveloperConsoleCommands(int *outCount)
{
    if (outCount) *outCount = (int)(sizeof(COMMANDS) / sizeof(COMMANDS[0]));
    return COMMANDS;
}

static const ConsoleCommand *FindCommand(const char *name)
{
    int count;
    const ConsoleCommand *commands = DeveloperConsoleCommands(&count);
    for (int i = 0; i < count; ++i) if (strcmp(commands[i].name, name) == 0) return &commands[i];
    return NULL;
}

static ResourceType ParseResource(const char *name)
{
    if (strcmp(name, "green") == 0) return RESOURCE_GREEN_ORE;
    if (strcmp(name, "acid") == 0) return RESOURCE_ACID_ORE;
    if (strcmp(name, "crystal") == 0) return RESOURCE_CRYSTAL_ORE;
    if (strcmp(name, "slime") == 0) return RESOURCE_SLIME;
    return RESOURCE_COUNT;
}

static bool CommandHelp(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize)
{
    if (argc == 0) {
        int count; const ConsoleCommand *commands = DeveloperConsoleCommands(&count);
        DeveloperConsoleWrite(context->console, "Commands:");
        for (int i = 0; i < count; ++i) DeveloperConsoleWrite(context->console, "  %s - %s", commands[i].usage, commands[i].description);
        snprintf(message, (size_t)messageSize, "Listed %d commands", count);
        return true;
    }
    const ConsoleCommand *command = FindCommand(argv[0]);
    if (!command) { snprintf(message, (size_t)messageSize, "Unknown command: %s", argv[0]); return false; }
    DeveloperConsoleWrite(context->console, "%s", command->usage);
    DeveloperConsoleWrite(context->console, "%s", command->description);
    if (command->parameters[0]) DeveloperConsoleWrite(context->console, "Parameters: %s", command->parameters);
    snprintf(message, (size_t)messageSize, "Help: %s", command->name);
    return true;
}

static bool CommandGive(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize)
{
    (void)argc;
    ResourceType resource = ParseResource(argv[0]);
    char *end = NULL;
    float amount = strtof(argv[1], &end);
    if (resource == RESOURCE_COUNT || !end || *end != '\0' || amount <= 0.0f) {
        snprintf(message, (size_t)messageSize, "Usage: give <green|acid|crystal|slime> <positive amount>");
        return false;
    }
    EconomyAdd(&context->state->economy, resource, amount);
    snprintf(message, (size_t)messageSize, "Added %.1f %s", amount, EconomyResourceName(resource));
    return true;
}

static bool CommandTeleport(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize)
{
    (void)argc;
    Vector3 position;
    if (!LevelGetLandmark(context->level, argv[0], &position)) {
        snprintf(message, (size_t)messageSize, "Unknown location. Use hub, ranch, lab, acid, crystal, range");
        return false;
    }
    context->playerSystem->player.position = position;
    context->playerSystem->player.velocity = Vector2Zero();
    snprintf(message, (size_t)messageSize, "Teleported to %s", argv[0]);
    return true;
}

static bool CommandSave(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize)
{ (void)argc; (void)argv; bool ok = SaveGameWrite(context->state, &context->playerSystem->player); snprintf(message, (size_t)messageSize, ok ? "Saved progress, player position and view" : "Save failed"); return ok; }

static bool CommandLoad(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize)
{
    (void)argc; (void)argv;
    bool ok = SaveGameRead(context->state, &context->playerSystem->player, context->level);
    if (ok) PlayerCameraUpdate(&context->playerSystem->camera, &context->playerSystem->player);
    snprintf(message, (size_t)messageSize, ok ? "Loaded progress, player position and view" : "No valid save");
    return ok;
}

static bool CommandUnlockZone(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize)
{ (void)argc; (void)argv; bool ok = GameStateTryUnlockZoneTwo(context->state); snprintf(message, (size_t)messageSize, ok ? "Acid Works unlocked" : "Need 55 slime or already unlocked"); return ok; }

static bool CommandContract(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize)
{
    (void)argc; (void)argv;
    DeveloperConsoleWrite(context->console, "NEXT: %s", ContractsObjective(&context->state->contracts));
    DeveloperConsoleWrite(context->console, "%s", ContractsDetail(&context->state->contracts, &context->state->economy, &context->state->buildings));
    snprintf(message, (size_t)messageSize, "Contract written to console");
    return true;
}

static bool CommandClear(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize)
{ (void)argc; (void)argv; DeveloperConsoleClear(context->console); snprintf(message, (size_t)messageSize, ""); return true; }
