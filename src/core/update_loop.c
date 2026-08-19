#include "update_loop.h"

void UpdateLoopInit(UpdateLoop *loop)
{
    *loop = (UpdateLoop){ 0 };
}

bool UpdateLoopRegister(UpdateLoop *loop, UpdateFunction update, void *context)
{
    if (!update || loop->count >= UPDATE_LOOP_MAX_SYSTEMS) return false;
    loop->entries[loop->count++] = (UpdateEntry){ update, context };
    return true;
}

void UpdateLoopRun(UpdateLoop *loop, float deltaTime)
{
    for (int i = 0; i < loop->count; ++i)
    { 
        loop->entries[i].update(loop->entries[i].context, deltaTime);
    }
}
