#ifndef SLIMEWORKS_UPDATE_LOOP_H
#define SLIMEWORKS_UPDATE_LOOP_H

#include <stdbool.h>

#define UPDATE_LOOP_MAX_SYSTEMS 8

typedef void (*UpdateFunction)(void *context, float deltaTime);

typedef struct UpdateEntry {
    UpdateFunction update;
    void *context;
} UpdateEntry;

typedef struct UpdateLoop {
    UpdateEntry entries[UPDATE_LOOP_MAX_SYSTEMS];
    int count;
} UpdateLoop;

void UpdateLoopInit(UpdateLoop *loop);
bool UpdateLoopRegister(UpdateLoop *loop, UpdateFunction update, void *context);
void UpdateLoopRun(UpdateLoop *loop, float deltaTime);

#endif
