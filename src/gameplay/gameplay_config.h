#ifndef SLIMEWORKS_GAMEPLAY_CONFIG_H
#define SLIMEWORKS_GAMEPLAY_CONFIG_H

#include <raylib.h>

typedef struct GameplayBindings {
    KeyboardKey toggleBuild;
    KeyboardKey buildPump;
    KeyboardKey buildRefinery;
    KeyboardKey buildMixer;
    KeyboardKey buildBooster;
    KeyboardKey buildPipe;
    KeyboardKey placeOrInteract;
    KeyboardKey removePipe;
    KeyboardKey upgrade;
    KeyboardKey unlockZone;
    KeyboardKey toggleHelp;
    KeyboardKey save;
    KeyboardKey load;
} GameplayBindings;

typedef struct GameplayRules {
    Vector3 gatePosition;
    float gateInteractionRadius;
    float buildWorldLimit;
    float pumpSourceRadius;
    float interactionRange;
} GameplayRules;

static inline GameplayBindings GameplayDefaultBindings(void)
{
    return (GameplayBindings){ KEY_B, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE,
                               KEY_E, KEY_R, KEY_U, KEY_O, KEY_F1, KEY_F5, KEY_F9 };
}

static inline GameplayRules GameplayDefaultRules(void)
{
    return (GameplayRules){ .gatePosition = { 20.0f, 0.0f, -3.0f }, .gateInteractionRadius = 5.7f,
                            .buildWorldLimit = 58.0f, .pumpSourceRadius = 3.2f, .interactionRange = 4.0f };
}

#endif
