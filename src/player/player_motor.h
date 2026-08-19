#ifndef SLIMEWORKS_PLAYER_MOTOR_H
#define SLIMEWORKS_PLAYER_MOTOR_H

#include "player_input.h"
#include "player_state.h"
#include "../world/level.h"

void PlayerMotorUpdate(Player *player, const PlayerInput *input, const Level *level, float deltaTime);

#endif
