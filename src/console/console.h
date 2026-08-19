#ifndef SLIMEWORKS_CONSOLE_H
#define SLIMEWORKS_CONSOLE_H

#include <raylib.h>
#include "../core/input_gate.h"
#include "../core/game_state.h"
#include "../gameplay/gameplay_system.h"
#include "../player/player_system.h"
#include "../world/level.h"

#define CONSOLE_INPUT_MAX 192
#define CONSOLE_HISTORY_MAX 24
#define CONSOLE_LOG_MAX 18
#define CONSOLE_LINE_MAX 160

typedef struct DeveloperConsole DeveloperConsole;

typedef struct ConsoleExecutionContext {
    GameState *state;
    Level *level;
    PlayerSystem *playerSystem;
    GameplaySystem *gameplay;
    DeveloperConsole *console;
} ConsoleExecutionContext;

typedef bool (*ConsoleCommandHandler)(ConsoleExecutionContext *context, int argc, const char *argv[], char *message, int messageSize);

typedef struct ConsoleCommand {
    const char *name;
    const char *description;
    const char *usage;
    const char *parameters;
    int minArgs;
    int maxArgs;
    ConsoleCommandHandler handler;
} ConsoleCommand;

struct DeveloperConsole {
    InputGate *inputGate;
    ConsoleExecutionContext context;
    bool open;
    char input[CONSOLE_INPUT_MAX];
    int inputLength;
    int selectedSuggestion;
    int historyIndex;
    char history[CONSOLE_HISTORY_MAX][CONSOLE_INPUT_MAX];
    int historyCount;
    char log[CONSOLE_LOG_MAX][CONSOLE_LINE_MAX];
    int logCount;
};

void DeveloperConsoleInit(DeveloperConsole *console, InputGate *inputGate, GameState *state, Level *level,
                          PlayerSystem *playerSystem, GameplaySystem *gameplay);
void DeveloperConsoleSystemUpdate(void *context, float deltaTime);
void DeveloperConsoleDraw(const DeveloperConsole *console);
void DeveloperConsoleWrite(DeveloperConsole *console, const char *format, ...);
void DeveloperConsoleClear(DeveloperConsole *console);
const ConsoleCommand *DeveloperConsoleCommands(int *outCount);

#endif
