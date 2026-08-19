#ifndef LEVEL_H
#define LEVEL_H

#include <raylib.h>

#define LEVEL_MAX_PLATFORMS 12
#define LEVEL_MAX_RAMPS 12
#define LEVEL_MAX_BOXES 32

typedef enum GroundType { GROUND_FLAT, GROUND_STEP, GROUND_RAMP } GroundType;
typedef struct GroundSample { float height; float slopeDegrees; GroundType type; } GroundSample;
typedef struct LevelPlatform { Vector3 center; Vector2 size; float top; Color color; } LevelPlatform;
typedef struct LevelRamp { Vector3 start; Vector3 end; float width; Color color; } LevelRamp;
typedef struct LevelBox { Vector3 position, size; Color color; bool collidable; } LevelBox;

typedef struct Level {
    bool zoneTwoUnlocked;
    LevelPlatform platforms[LEVEL_MAX_PLATFORMS];
    int platformCount;
    LevelRamp ramps[LEVEL_MAX_RAMPS];
    int rampCount;
    LevelBox boxes[LEVEL_MAX_BOXES];
    int boxCount;
    int gateColliderIndex;
} Level;

void LevelInit(Level *level);
void LevelShutdown(Level *level);
void LevelDraw(const Level *level);
Vector3 LevelGetSpawnPoint(const Level *level);
bool LevelGetLandmark(const Level *level, const char *name, Vector3 *outPosition);
float LevelGroundHeight(const Level *level, float x, float z);
GroundSample LevelSampleGround(const Level *level, float x, float z);
bool LevelCollides(const Level *level, float x, float z, float radius);

#endif
