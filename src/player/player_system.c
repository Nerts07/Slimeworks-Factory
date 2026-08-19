#include "player_system.h"

void PlayerSystemInit(PlayerSystem *system, const Level *level, const InputGate *inputGate, Vector3 spawn)
{
    *system = (PlayerSystem){ .level = level, .inputGate = inputGate, .mouseCaptured = true, .bindings = PlayerDefaultBindings() };
    PlayerInit(&system->player, spawn);
    system->camera = (Camera3D){ .projection = CAMERA_PERSPECTIVE, .fovy = 85.0f, .up = (Vector3){ 0, 1, 0 } };
    DisableCursor();
    PlayerCameraUpdate(&system->camera, &system->player);
}

void PlayerSystemUpdate(void *context, float deltaTime)
{
    PlayerSystem *system = context;
    if (system->inputGate && system->inputGate->gameplayBlocked) {
        system->input = (PlayerInput){ 0 };
        PlayerCameraUpdate(&system->camera, &system->player);
        return;
    }
    PlayerInputUpdate(&system->input, &system->bindings, system->mouseCaptured);
    if (system->input.toggleCursorPressed) {
        system->mouseCaptured = !system->mouseCaptured;
        if (system->mouseCaptured) DisableCursor(); else EnableCursor();
    }
    if (system->mouseCaptured) PlayerCameraApplyLook(&system->player, system->input.mouseDelta);
    PlayerMotorUpdate(&system->player, &system->input, system->level, deltaTime);
    PlayerCameraUpdate(&system->camera, &system->player);
}
