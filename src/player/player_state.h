#ifndef SLIMEWORKS_PLAYER_STATE_H
#define SLIMEWORKS_PLAYER_STATE_H

#include <raylib.h>
#include "player_config.h"

typedef struct Player {
    Vector3 position;
    Vector2 velocity;
    float verticalVelocity;
    float yaw;
    float pitch;
    bool grounded;
    PlayerConfig config;
} Player;

void PlayerInit(Player *player, Vector3 spawn);
Vector3 PlayerEyePosition(const Player *player);
Vector3 PlayerLookDirection(const Player *player);

#endif
