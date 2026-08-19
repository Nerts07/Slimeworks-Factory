#include "console.h"
#include <raymath.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void ConsoleSetOpen(DeveloperConsole *console, bool open)
{
    console->open = open;
    console->inputGate->gameplayBlocked = open;
    console->selectedSuggestion = 0;
    console->historyIndex = -1;
    if (open) DeveloperConsoleWrite(console, "Slimeworks developer console — type help for commands");
}

void DeveloperConsoleInit(DeveloperConsole *console, InputGate *inputGate, GameState *state, Level *level,
                          PlayerSystem *playerSystem, GameplaySystem *gameplay)
{
    *console = (DeveloperConsole){ .inputGate = inputGate, .historyIndex = -1 };
    console->context = (ConsoleExecutionContext){ state, level, playerSystem, gameplay, console };
}

void DeveloperConsoleClear(DeveloperConsole *console)
{
    console->logCount = 0;
}

void DeveloperConsoleWrite(DeveloperConsole *console, const char *format, ...)
{
    if (console->logCount == CONSOLE_LOG_MAX) {
        memmove(console->log, console->log + 1, (CONSOLE_LOG_MAX - 1) * CONSOLE_LINE_MAX);
        console->logCount--;
    }
    va_list args;
    va_start(args, format);
    vsnprintf(console->log[console->logCount++], CONSOLE_LINE_MAX, format, args);
    va_end(args);
}

static int MatchCommands(const DeveloperConsole *console, const ConsoleCommand **matches, int maxMatches)
{
    char word[CONSOLE_INPUT_MAX] = { 0 };
    int length = 0;
    while (length < console->inputLength && !isspace((unsigned char)console->input[length])) { word[length] = console->input[length]; length++; }
    int commandCount, found = 0;
    const ConsoleCommand *commands = DeveloperConsoleCommands(&commandCount);
    for (int i = 0; i < commandCount && found < maxMatches; ++i)
        if (length == 0 || strncmp(commands[i].name, word, (size_t)length) == 0) matches[found++] = &commands[i];
    return found;
}

static bool InputHasArguments(const DeveloperConsole *console)
{
    for (int i = 0; i < console->inputLength; ++i) if (isspace((unsigned char)console->input[i])) return true;
    return false;
}

static void ApplySuggestion(DeveloperConsole *console)
{
    const ConsoleCommand *matches[16];
    int count = MatchCommands(console, matches, 16);
    if (count <= 0) return;
    const char *name = matches[console->selectedSuggestion % count]->name;
    snprintf(console->input, sizeof(console->input), "%s ", name);
    console->inputLength = (int)strlen(console->input);
}

static void PushHistory(DeveloperConsole *console)
{
    if (console->inputLength == 0) return;
    if (console->historyCount == CONSOLE_HISTORY_MAX) {
        memmove(console->history, console->history + 1, (CONSOLE_HISTORY_MAX - 1) * CONSOLE_INPUT_MAX);
        console->historyCount--;
    }
    snprintf(console->history[console->historyCount++], CONSOLE_INPUT_MAX, "%s", console->input);
    console->historyIndex = -1;
}

static void ExecuteInput(DeveloperConsole *console)
{
    char working[CONSOLE_INPUT_MAX];
    snprintf(working, sizeof(working), "%s", console->input);
    const char *argv[8];
    int argc = 0;
    char *token = strtok(working, " \t");
    while (token && argc < 8) { argv[argc++] = token; token = strtok(NULL, " \t"); }
    if (argc == 0) return;
    DeveloperConsoleWrite(console, "> %s", console->input);
    PushHistory(console);
    int count;
    const ConsoleCommand *commands = DeveloperConsoleCommands(&count);
    const ConsoleCommand *command = NULL;
    for (int i = 0; i < count; ++i) if (strcmp(commands[i].name, argv[0]) == 0) { command = &commands[i]; break; }
    char message[CONSOLE_LINE_MAX];
    if (!command) snprintf(message, sizeof(message), "Unknown command: %s", argv[0]);
    else if (argc - 1 < command->minArgs || argc - 1 > command->maxArgs)
        snprintf(message, sizeof(message), "Usage: %s", command->usage);
    else command->handler(&console->context, argc - 1, argv + 1, message, sizeof(message));
    if (message[0]) DeveloperConsoleWrite(console, "%s", message);
    console->input[0] = '\0';
    console->inputLength = 0;
    console->selectedSuggestion = 0;
}

static void NavigateHistory(DeveloperConsole *console, int direction)
{
    if (console->historyCount == 0) return;
    if (console->historyIndex < 0) console->historyIndex = console->historyCount;
    console->historyIndex = Clamp(console->historyIndex + direction, 0, console->historyCount - 1);
    snprintf(console->input, sizeof(console->input), "%s", console->history[console->historyIndex]);
    console->inputLength = (int)strlen(console->input);
}

void DeveloperConsoleSystemUpdate(void *context, float deltaTime)
{
    (void)deltaTime;
    DeveloperConsole *console = context;
    if (IsKeyPressed(KEY_F4)) { ConsoleSetOpen(console, !console->open); return; }
    if (!console->open) return;
    if (IsKeyPressed(KEY_ESCAPE)) { ConsoleSetOpen(console, false); return; }
    if (IsKeyPressed(KEY_BACKSPACE) && console->inputLength > 0) console->input[--console->inputLength] = '\0';
    if (IsKeyPressed(KEY_TAB)) ApplySuggestion(console);
    /* Empty input is history navigation; typing a command prefix switches arrows to completion. */
    bool browsingSuggestions = console->inputLength > 0 && !InputHasArguments(console);
    if (IsKeyPressed(KEY_UP)) {
        if (browsingSuggestions) console->selectedSuggestion = console->selectedSuggestion > 0 ? console->selectedSuggestion - 1 : 0;
        else NavigateHistory(console, -1);
    }
    if (IsKeyPressed(KEY_DOWN)) {
        if (browsingSuggestions) console->selectedSuggestion++;
        else NavigateHistory(console, 1);
    }
    if (IsKeyPressed(KEY_ENTER)) ExecuteInput(console);
    int codepoint = GetCharPressed();
    while (codepoint > 0) {
        if (codepoint >= 32 && codepoint < 127 && console->inputLength < CONSOLE_INPUT_MAX - 1) {
            console->input[console->inputLength++] = (char)codepoint;
            console->input[console->inputLength] = '\0';
            console->selectedSuggestion = 0;
        }
        codepoint = GetCharPressed();
    }
}

static void DrawHighlightedInput(const DeveloperConsole *console, int x, int y)
{
    int commandLength = 0;
    while (commandLength < console->inputLength && !isspace((unsigned char)console->input[commandLength])) commandLength++;
    char command[CONSOLE_INPUT_MAX] = { 0 };
    memcpy(command, console->input, (size_t)commandLength);
    int count; const ConsoleCommand *commands = DeveloperConsoleCommands(&count);
    bool valid = false;
    for (int i = 0; i < count; ++i) if (strcmp(command, commands[i].name) == 0) valid = true;
    DrawText("]", x, y, 20, SKYBLUE);
    DrawText(command, x + 18, y, 20, commandLength == 0 ? LIGHTGRAY : (valid ? LIME : ORANGE));
    DrawText(console->input + commandLength, x + 18 + MeasureText(command, 20), y, 20, RAYWHITE);
    if (((int)(GetTime() * 2.0) % 2) == 0) DrawRectangle(x + 20 + MeasureText(console->input, 20), y + 2, 2, 20, RAYWHITE);
}

void DeveloperConsoleDraw(const DeveloperConsole *console)
{
    if (!console->open) return;
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.78f));
    DrawRectangle(22, 22, sw - 44, sh - 44, (Color){ 17, 23, 30, 245 });
    DrawRectangleLines(22, 22, sw - 44, sh - 44, Fade(SKYBLUE, 0.85f));
    DrawText("SLIMEWORKS DEVELOPER CONSOLE  //  F4 or ESC closes  //  TAB completes", 42, 40, 19, SKYBLUE);
    int logY = 76;
    for (int i = 0; i < console->logCount; ++i) DrawText(console->log[i], 44, logY + i * 20, 16, LIGHTGRAY);
    int suggestionY = sh - 170;
    const ConsoleCommand *matches[8];
    int matchCount = MatchCommands(console, matches, 8);
    for (int i = 0; i < matchCount; ++i) {
        Color color = i == (console->selectedSuggestion % matchCount) ? GOLD : LIGHTGRAY;
        DrawText(matches[i]->usage, 44, suggestionY + i * 20, 16, color);
        DrawText(matches[i]->description, 280, suggestionY + i * 20, 16, Fade(color, 0.8f));
    }
    const ConsoleCommand *selected = matchCount > 0 ? matches[console->selectedSuggestion % matchCount] : NULL;
    if (selected) {
        DrawText(selected->parameters, sw - 460, sh - 92, 15, SKYBLUE);
        DrawText("UP/DOWN: select command or history", sw - 460, sh - 66, 14, GRAY);
    }
    DrawRectangle(34, sh - 54, sw - 68, 34, Fade(BLACK, 0.5f));
    DrawHighlightedInput(console, 46, sh - 48);
}
