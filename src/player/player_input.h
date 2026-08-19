#ifndef SLIMEWORKS_PLAYER_INPUT_H
#define SLIMEWORKS_PLAYER_INPUT_H

#include <raylib.h>

typedef struct PlayerInput {
    Vector2 move;
    Vector2 mouseDelta;
    bool sprintHeld;
    bool jumpPressed;
    bool toggleCursorPressed;
} PlayerInput;

typedef struct PlayerBindings {
    KeyboardKey moveLeft;
    KeyboardKey moveRight;
    KeyboardKey moveForward;
    KeyboardKey moveBackward;
    KeyboardKey sprint;
    KeyboardKey jump;
    KeyboardKey toggleCursor;
} PlayerBindings;

static inline PlayerBindings PlayerDefaultBindings(void)
{
    return (PlayerBindings){ KEY_A, KEY_D, KEY_W, KEY_S, KEY_LEFT_SHIFT, KEY_SPACE, KEY_ESCAPE };
}

void PlayerInputUpdate(PlayerInput *input, const PlayerBindings *bindings, bool inputEnabled);

#endif
