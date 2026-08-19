#ifndef SLIMEWORKS_PLAYER_SYSTEM_H
#define SLIMEWORKS_PLAYER_SYSTEM_H

#include "player_camera.h"
#include "player_input.h"
#include "player_motor.h"
#include "../core/input_gate.h"

typedef struct PlayerSystem {
    Player player;
    PlayerInput input;
    PlayerBindings bindings;
    Camera3D camera;
    const Level *level;
    const InputGate *inputGate;
    bool mouseCaptured;
} PlayerSystem;

void PlayerSystemInit(PlayerSystem *system, const Level *level, const InputGate *inputGate, Vector3 spawn);
void PlayerSystemUpdate(void *context, float deltaTime);

#endif
