#include "player_input.h"

void PlayerInputUpdate(PlayerInput *input, const PlayerBindings *bindings, bool inputEnabled)
{
    *input = (PlayerInput){ .toggleCursorPressed = IsKeyPressed(bindings->toggleCursor) };
    if (!inputEnabled) return;
    input->move = (Vector2){ (float)IsKeyDown(bindings->moveRight) - (float)IsKeyDown(bindings->moveLeft),
                             (float)IsKeyDown(bindings->moveForward) - (float)IsKeyDown(bindings->moveBackward) };
    input->mouseDelta = GetMouseDelta();
    input->sprintHeld = IsKeyDown(bindings->sprint);
    input->jumpPressed = IsKeyPressed(bindings->jump);
}
