#include "player_state.h"
#include <math.h>

void PlayerInit(Player *player, Vector3 spawn)
{
    *player = (Player){ 0 };
    player->position = spawn;
    player->config = PlayerDefaultConfig();
    player->grounded = true;
}

Vector3 PlayerEyePosition(const Player *player)
{
    return (Vector3){ player->position.x, player->position.y + player->config.eyeHeight, player->position.z };
}

Vector3 PlayerLookDirection(const Player *player)
{
    float cosPitch = cosf(player->pitch);
    return (Vector3){ sinf(player->yaw) * cosPitch, sinf(player->pitch), cosf(player->yaw) * cosPitch };
}
