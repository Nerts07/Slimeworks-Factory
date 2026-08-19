#ifndef PLAYER_CONFIG_H
#define PLAYER_CONFIG_H

/* Tunable movement values live here so game feel can be changed without
   touching the controller implementation. Units are world units/seconds. */
typedef struct PlayerConfig {
    float walkSpeed;
    float sprintSpeed;
    float acceleration;
    float deceleration;
    float mouseSensitivity;
    float eyeHeight;
    float radius;
    float maxStepHeight;       /* maximum vertical discontinuity to auto-step */
    float stepSmoothingSpeed;
    float gravity;
    float jumpSpeed;
    float slopeLimitDegrees;   /* maximum walkable ramp incline */
} PlayerConfig;

static inline PlayerConfig PlayerDefaultConfig(void)
{
    return (PlayerConfig){
        .walkSpeed = 4.2f, .sprintSpeed = 7.0f, .acceleration = 24.0f,
        .deceleration = 30.0f, .mouseSensitivity = 0.0025f, .eyeHeight = 1.72f,
        .radius = 0.34f, .maxStepHeight = 0.30f, .stepSmoothingSpeed = 7.5f,
        .gravity = 25.6f, .jumpSpeed = 8.0f, .slopeLimitDegrees = 48.0f
    };
}

#endif
