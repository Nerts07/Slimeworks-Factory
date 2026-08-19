#ifndef SLIMEWORKS_UI_H
#define SLIMEWORKS_UI_H

#include "../core/game_state.h"
#include "../player/player.h"

void UiDrawHud(const Player *player, const GameState *state, bool mouseCaptured, bool buildMode,
               BuildType selectedBuild, const ResourceNode *targetNode, const Building *nearbyBuilding,
               float harvestProgress, const char *notification, float notificationSeconds, bool helpOpen);

#endif
