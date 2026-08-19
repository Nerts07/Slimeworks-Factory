#include "player_motor.h"
#include <math.h>
#include <raymath.h>

static float Approach(float current, float target, float amount)
{
    return current < target ? fminf(current + amount, target) : fmaxf(current - amount, target);
}

static bool CanTraverseGround(const Player *player, const Level *level, float targetX, float targetZ)
{
    GroundSample current = LevelSampleGround(level, player->position.x, player->position.z);
    GroundSample target = LevelSampleGround(level, targetX, targetZ);
    float rise = target.height - current.height;
    if (rise <= 0.001f) return true;
    if (target.type == GROUND_RAMP) return target.slopeDegrees <= player->config.slopeLimitDegrees;
    return rise <= player->config.maxStepHeight + 0.001f;
}

static bool TryMoveHorizontal(Player *player, const Level *level, float x, float z)
{
    if (LevelCollides(level, x, z, player->config.radius) || !CanTraverseGround(player, level, x, z)) return false;
    player->position.x = x;
    player->position.z = z;
    return true;
}

void PlayerMotorUpdate(Player *player, const PlayerInput *input, const Level *level, float deltaTime)
{
    Vector2 move = input->move;
    if (Vector2Length(move) > 1.0f) move = Vector2Normalize(move);
    float speed = input->sprintHeld ? player->config.sprintSpeed : player->config.walkSpeed;
    Vector2 forward = { sinf(player->yaw), cosf(player->yaw) };
    Vector2 right = { -cosf(player->yaw), sinf(player->yaw) };
    Vector2 desired = Vector2Add(Vector2Scale(forward, move.y), Vector2Scale(right, move.x));
    desired = Vector2Scale(desired, speed);
    float rate = Vector2Length(desired) > 0.01f ? player->config.acceleration : player->config.deceleration;
    player->velocity.x = Approach(player->velocity.x, desired.x, rate * deltaTime);
    player->velocity.y = Approach(player->velocity.y, desired.y, rate * deltaTime);

    if (player->grounded && input->jumpPressed) {
        player->grounded = false;
        player->verticalVelocity = player->config.jumpSpeed;
    }

    Vector3 requested = { player->position.x + player->velocity.x * deltaTime, player->position.y,
                          player->position.z + player->velocity.y * deltaTime };
    if (!TryMoveHorizontal(player, level, requested.x, requested.z)) {
        TryMoveHorizontal(player, level, requested.x, player->position.z);
        TryMoveHorizontal(player, level, player->position.x, requested.z);
    }

    float ground = LevelGroundHeight(level, player->position.x, player->position.z);
    if (player->grounded && player->position.y - ground > player->config.maxStepHeight + 0.05f) {
        player->grounded = false;
        player->verticalVelocity = 0.0f;
    }
    if (!player->grounded) {
        player->verticalVelocity -= player->config.gravity * deltaTime;
        player->position.y += player->verticalVelocity * deltaTime;
        if (player->position.y <= ground) { player->position.y = ground; player->grounded = true; }
    } else if (ground > player->position.y) {
        player->position.y = fminf(player->position.y + player->config.stepSmoothingSpeed * deltaTime, ground);
    } else {
        player->position.y = fmaxf(player->position.y - player->config.stepSmoothingSpeed * deltaTime, ground);
    }
}
