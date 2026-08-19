#include "player_camera.h"
#include <raymath.h>

void PlayerCameraApplyLook(Player *player, Vector2 mouseDelta)
{
    player->yaw -= mouseDelta.x * player->config.mouseSensitivity;
    player->pitch = Clamp(player->pitch - mouseDelta.y * player->config.mouseSensitivity, -1.45f, 1.45f);
}

void PlayerCameraUpdate(Camera3D *camera, const Player *player)
{
    camera->position = PlayerEyePosition(player);
    camera->target = Vector3Add(camera->position, PlayerLookDirection(player));
}
