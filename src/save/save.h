#ifndef SLIMEWORKS_SAVE_H
#define SLIMEWORKS_SAVE_H

#include <stdbool.h>
#include "../core/game_state.h"
#include "../player/player_state.h"

bool SaveGameWrite(const GameState *state, const Player *player);
bool SaveGameRead(GameState *state, Player *player, const Level *level);

#endif
