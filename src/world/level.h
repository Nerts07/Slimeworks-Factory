#ifndef LEVEL_H
#define LEVEL_H

#include <raylib.h>
#include "../render/game_visuals.h"

#define LEVEL_MAX_PLATFORMS 96
#define LEVEL_MAX_RAMPS 96
#define LEVEL_MAX_BOXES 256
#define LEVEL_MAX_MODELS 96
#define LEVEL_MAX_MESHES 128
#define LEVEL_MAX_MARKERS 128
#define LEVEL_NAME_LENGTH 32
#define LEVEL_TAG_LENGTH 32
#define LEVEL_ASSET_PATH_LENGTH 256
#define LEVEL_MESH_MAX_VERTICES 128
#define LEVEL_MESH_MAX_FACES 128
#define LEVEL_MESH_FACE_MAX_VERTICES 4
#define LEVEL_TITLE_LENGTH 64

typedef enum GroundType { GROUND_FLAT, GROUND_STEP, GROUND_RAMP } GroundType;
typedef struct GroundSample { float height; float slopeDegrees; GroundType type; } GroundSample;
typedef struct LevelPlatform {
    char name[LEVEL_NAME_LENGTH];
    char tag[LEVEL_TAG_LENGTH];
    Vector3 center;
    Vector2 size;
    float top;
    Color color;
} LevelPlatform;

typedef struct LevelRamp {
    char name[LEVEL_NAME_LENGTH];
    char tag[LEVEL_TAG_LENGTH];
    Vector3 start;
    Vector3 end;
    float width;
    Color color;
} LevelRamp;

typedef struct LevelBox {
    char name[LEVEL_NAME_LENGTH];
    char tag[LEVEL_TAG_LENGTH];
    Vector3 position;
    Vector3 size;
    Vector3 rotation;
    Color faceColors[6];
    float bevel;
    int subdivisions;
    bool collidable;
    bool visible;
} LevelBox;

typedef struct LevelModel {
    char name[LEVEL_NAME_LENGTH];
    char tag[LEVEL_TAG_LENGTH];
    char assetPath[LEVEL_ASSET_PATH_LENGTH];
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Vector3 collisionSize;
    bool collidable;
    Model model;
    bool modelLoaded;
} LevelModel;

typedef struct LevelMeshFace {
    unsigned char vertexCount;
    unsigned short vertices[LEVEL_MESH_FACE_MAX_VERTICES];
    Color color;
} LevelMeshFace;

typedef struct LevelMesh {
    char name[LEVEL_NAME_LENGTH];
    char tag[LEVEL_TAG_LENGTH];
    Vector3 position;
    Vector3 vertices[LEVEL_MESH_MAX_VERTICES];
    int vertexCount;
    LevelMeshFace faces[LEVEL_MESH_MAX_FACES];
    int faceCount;
    bool collidable;
} LevelMesh;

typedef struct LevelMarker {
    char name[LEVEL_NAME_LENGTH];
    char tag[LEVEL_TAG_LENGTH];
    Vector3 position;
} LevelMarker;

typedef struct Level {
    char title[LEVEL_TITLE_LENGTH];
    unsigned int seed;
    bool generated;
    bool zoneTwoUnlocked;
    LevelPlatform platforms[LEVEL_MAX_PLATFORMS];
    int platformCount;
    LevelRamp ramps[LEVEL_MAX_RAMPS];
    int rampCount;
    LevelBox boxes[LEVEL_MAX_BOXES];
    int boxCount;
    LevelModel models[LEVEL_MAX_MODELS];
    int modelCount;
    LevelMesh meshes[LEVEL_MAX_MESHES];
    int meshCount;
    LevelMarker markers[LEVEL_MAX_MARKERS];
    int markerCount;
    int gateColliderIndex;
} Level;

void LevelInit(Level *level);
/* Builds a replayable procedural expedition from a seed. */
void LevelGenerate(Level *level, unsigned int seed);
void LevelShutdown(Level *level);
bool LevelLoadFromFile(Level *level, const char *filePath);
void LevelDraw(const Level *level, const GameVisuals *visuals);
Vector3 LevelGetSpawnPoint(const Level *level);
bool LevelGetLandmark(const Level *level, const char *name, Vector3 *outPosition);
bool LevelFindMarker(const Level *level, const char *nameOrTag, Vector3 *outPosition);
int LevelFindMarkersWithTag(const Level *level, const char *tag, Vector3 *outPositions, int maxPositions);
const LevelModel *LevelFindModel(const Level *level, const char *nameOrTag);
int LevelFindModelsWithTag(const Level *level, const char *tag, const LevelModel **outModels, int maxModels);
float LevelGroundHeight(const Level *level, float x, float z);
GroundSample LevelSampleGround(const Level *level, float x, float z);
bool LevelCollides(const Level *level, float x, float z, float radius);

#endif
