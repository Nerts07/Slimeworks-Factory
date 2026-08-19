#ifndef SLIMEWORKS_PLAYER_CAMERA_H
#define SLIMEWORKS_PLAYER_CAMERA_H

#include <raylib.h>
#include "player_state.h"

void PlayerCameraApplyLook(Player *player, Vector2 mouseDelta);
void PlayerCameraUpdate(Camera3D *camera, const Player *player);

#endif
