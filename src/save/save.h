#ifndef SLIMEWORKS_SAVE_H
#define SLIMEWORKS_SAVE_H

#include <stdbool.h>
#include "../core/game_state.h"

bool SaveGameWrite(const GameState *state);
bool SaveGameRead(GameState *state);

#endif
