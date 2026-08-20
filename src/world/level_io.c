#include "level_io.h"

#include <math.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void StripLine(char *line)
{
    line[strcspn(line, "\r\n")] = '\0';
}

static void CopyString(char *destination, size_t destinationSize, const char *source)
{
    snprintf(destination, destinationSize, "%s", source);
}

static void GetLevelDirectory(const char *filePath, char *directory, size_t directorySize)
{
    const char *slash = strrchr(filePath, '/');
    if (!slash) { CopyString(directory, directorySize, "."); return; }
    size_t length = (size_t)(slash - filePath);
    if (length >= directorySize) length = directorySize - 1;
    memcpy(directory, filePath, length);
    directory[length] = '\0';
}

static bool ParseFaceColors(const char *line, Color faceColors[6])
{
    char buffer[512];
    CopyString(buffer, sizeof(buffer), line);
    char *token = strtok(buffer, " \t");
    if (!token || strcmp(token, "FACES") != 0) return false;
    for (int face = 0; face < 6; ++face) {
        int rgba[4];
        for (int component = 0; component < 4; ++component) {
            token = strtok(NULL, " \t");
            if (!token) return false;
            rgba[component] = atoi(token);
            if (rgba[component] < 0 || rgba[component] > 255) return false;
        }
        faceColors[face] = (Color){ (unsigned char)rgba[0], (unsigned char)rgba[1],
                                   (unsigned char)rgba[2], (unsigned char)rgba[3] };
    }
    return true;
}

static void EnsureMeshOutwardWinding(LevelMesh *mesh)
{
    if (!mesh || mesh->vertexCount < 3) return;
    Vector3 meshCenter = { 0 };
    for (int i = 0; i < mesh->vertexCount; ++i) meshCenter = Vector3Add(meshCenter, mesh->vertices[i]);
    meshCenter = Vector3Scale(meshCenter, 1.0f / (float)mesh->vertexCount);
    for (int i = 0; i < mesh->faceCount; ++i) {
        LevelMeshFace *face = &mesh->faces[i];
        if (face->vertexCount < 3) continue;
        Vector3 a = mesh->vertices[face->vertices[0]];
        Vector3 b = mesh->vertices[face->vertices[1]];
        Vector3 c = mesh->vertices[face->vertices[2]];
        Vector3 normal = Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a));
        Vector3 faceCenter = { 0 };
        for (int vertex = 0; vertex < face->vertexCount; ++vertex)
            faceCenter = Vector3Add(faceCenter, mesh->vertices[face->vertices[vertex]]);
        faceCenter = Vector3Scale(faceCenter, 1.0f / (float)face->vertexCount);
        if (Vector3DotProduct(normal, Vector3Subtract(faceCenter, meshCenter)) < 0.0f) {
            for (int left = 1, right = face->vertexCount - 1; left < right; ++left, --right) {
                unsigned short swap = face->vertices[left];
                face->vertices[left] = face->vertices[right];
                face->vertices[right] = swap;
            }
        }
    }
}

static bool LoadModelAsset(LevelModel *model, const char *levelDirectory)
{
    char fullPath[512];
    if (model->assetPath[0] == '/') CopyString(fullPath, sizeof(fullPath), model->assetPath);
    else snprintf(fullPath, sizeof(fullPath), "%s/%s", levelDirectory, model->assetPath);
    if (!FileExists(fullPath)) {
        TraceLog(LOG_WARNING, "LEVL: model asset not found: %s", fullPath);
        return true;
    }
    model->model = LoadModel(fullPath);
    model->modelLoaded = IsModelValid(model->model);
    if (!model->modelLoaded) TraceLog(LOG_WARNING, "LEVL: unable to load model: %s", fullPath);
    return true;
}

bool LevelLoadFromFile(Level *level, const char *filePath)
{
    FILE *file = fopen(filePath, "r");
    if (!file) return false;

    Level parsed = { .gateColliderIndex = -1, .generated = false };
    CopyString(parsed.title, sizeof(parsed.title), GetFileNameWithoutExt(filePath));
    char directory[512];
    GetLevelDirectory(filePath, directory, sizeof(directory));
    char line[1024];
    bool validHeader = false;
    LevelBox *lastBox = NULL;

    while (fgets(line, sizeof(line), file)) {
        StripLine(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        if (strncmp(line, "PARENT ", 7) == 0 || strncmp(line, "EMPTY ", 6) == 0) continue;
        if (!validHeader) {
            int version = 0;
            if (sscanf(line, "SLIMEWORKS_LEVL %d", &version) != 1 || version != 1) goto parse_error;
            validHeader = true;
            continue;
        }

        if (strncmp(line, "LEVEL ", 6) == 0) {
            char levelName[LEVEL_TITLE_LENGTH];
            if (sscanf(line, "LEVEL %63s", levelName) == 1) CopyString(parsed.title, sizeof(parsed.title), levelName);
            continue;
        }
        if (strcmp(line, "END") == 0) break;

        if (strncmp(line, "PLATFORM ", 9) == 0) {
            if (parsed.platformCount >= LEVEL_MAX_PLATFORMS) goto parse_error;
            LevelPlatform *platform = &parsed.platforms[parsed.platformCount];
            int r, g, b, a;
            if (sscanf(line, "PLATFORM %31s %31s %f %f %f %f %f %d %d %d %d", platform->name, platform->tag,
                       &platform->center.x, &platform->center.z, &platform->size.x, &platform->size.y,
                       &platform->top, &r, &g, &b, &a) != 11) goto parse_error;
            if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255) goto parse_error;
            platform->center.y = 0.0f;
            platform->color = (Color){ (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a };
            parsed.platformCount++;
            lastBox = NULL;
            continue;
        }

        if (strncmp(line, "RAMP ", 5) == 0) {
            if (parsed.rampCount >= LEVEL_MAX_RAMPS) goto parse_error;
            LevelRamp *ramp = &parsed.ramps[parsed.rampCount];
            int r, g, b, a;
            if (sscanf(line, "RAMP %31s %31s %f %f %f %f %f %f %f %d %d %d %d", ramp->name, ramp->tag,
                       &ramp->start.x, &ramp->start.y, &ramp->start.z, &ramp->end.x, &ramp->end.y, &ramp->end.z,
                       &ramp->width, &r, &g, &b, &a) != 13) goto parse_error;
            if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255) goto parse_error;
            ramp->color = (Color){ (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a };
            parsed.rampCount++;
            lastBox = NULL;
            continue;
        }

        if (strncmp(line, "BOX ", 4) == 0) {
            if (parsed.boxCount >= LEVEL_MAX_BOXES) goto parse_error;
            LevelBox *box = &parsed.boxes[parsed.boxCount];
            int collidable;
            if (sscanf(line, "BOX %31s %31s %f %f %f %f %f %f %f %f %f %f %d %d", box->name, box->tag,
                       &box->position.x, &box->position.y, &box->position.z,
                       &box->size.x, &box->size.y, &box->size.z,
                       &box->rotation.x, &box->rotation.y, &box->rotation.z,
                       &box->bevel, &box->subdivisions, &collidable) != 14) goto parse_error;
            if (box->size.x <= 0.0f || box->size.y <= 0.0f || box->size.z <= 0.0f) goto parse_error;
            if (box->subdivisions < 1) box->subdivisions = 1;
            if (box->subdivisions > 16) box->subdivisions = 16;
            box->collidable = collidable != 0;
            box->visible = true;
            for (int i = 0; i < 6; ++i) box->faceColors[i] = LIGHTGRAY;
            if (strcmp(box->tag, "zone_two_gate") == 0) parsed.gateColliderIndex = parsed.boxCount;
            parsed.boxCount++;
            lastBox = box;
            continue;
        }

        if (strncmp(line, "FACES ", 6) == 0) {
            if (!lastBox || !ParseFaceColors(line, lastBox->faceColors)) goto parse_error;
            continue;
        }

        if (strncmp(line, "MESH ", 5) == 0) {
            if (parsed.meshCount >= LEVEL_MAX_MESHES) goto parse_error;
            LevelMesh *mesh = &parsed.meshes[parsed.meshCount];
            int collidable, vertexCount, faceCount;
            if (sscanf(line, "MESH %31s %31s %f %f %f %d %d %d", mesh->name, mesh->tag,
                       &mesh->position.x, &mesh->position.y, &mesh->position.z, &collidable, &vertexCount, &faceCount) != 8 ||
                vertexCount < 3 || vertexCount > LEVEL_MESH_MAX_VERTICES || faceCount < 1 || faceCount > LEVEL_MESH_MAX_FACES) goto parse_error;
            *mesh = (LevelMesh){ .position = mesh->position, .vertexCount = vertexCount, .faceCount = faceCount, .collidable = collidable != 0 };
            if (sscanf(line, "MESH %31s %31s", mesh->name, mesh->tag) != 2) goto parse_error;
            for (int vertex = 0; vertex < vertexCount; ++vertex) {
                if (!fgets(line, sizeof(line), file)) goto parse_error;
                StripLine(line);
                if (sscanf(line, "VERTEX %f %f %f", &mesh->vertices[vertex].x, &mesh->vertices[vertex].y, &mesh->vertices[vertex].z) != 3) goto parse_error;
            }
            for (int face = 0; face < faceCount; ++face) {
                int count, a, b, c, d, r, g, blue, alpha;
                if (!fgets(line, sizeof(line), file)) goto parse_error;
                StripLine(line);
                if (sscanf(line, "FACE %d %d %d %d %d %d %d %d %d", &count, &a, &b, &c, &d, &r, &g, &blue, &alpha) != 9 ||
                    count < 3 || count > LEVEL_MESH_FACE_MAX_VERTICES || a < 0 || b < 0 || c < 0 || d < 0 ||
                    a >= vertexCount || b >= vertexCount || c >= vertexCount || d >= vertexCount ||
                    r < 0 || r > 255 || g < 0 || g > 255 || blue < 0 || blue > 255 || alpha < 0 || alpha > 255) goto parse_error;
                mesh->faces[face] = (LevelMeshFace){ .vertexCount = (unsigned char)count,
                    .vertices = { (unsigned short)a, (unsigned short)b, (unsigned short)c, (unsigned short)d },
                    .color = { (unsigned char)r, (unsigned char)g, (unsigned char)blue, (unsigned char)alpha } };
            }
            if (!fgets(line, sizeof(line), file)) goto parse_error;
            StripLine(line);
            if (strcmp(line, "END_MESH") != 0) goto parse_error;
            EnsureMeshOutwardWinding(mesh);
            parsed.meshCount++;
            lastBox = NULL;
            continue;
        }

        if (strncmp(line, "MODEL ", 6) == 0) {
            if (parsed.modelCount >= LEVEL_MAX_MODELS) goto parse_error;
            LevelModel *model = &parsed.models[parsed.modelCount];
            int collidable;
            if (sscanf(line, "MODEL %31s %31s %255s %f %f %f %f %f %f %f %f %f %d %f %f %f",
                       model->name, model->tag, model->assetPath,
                       &model->position.x, &model->position.y, &model->position.z,
                       &model->rotation.x, &model->rotation.y, &model->rotation.z,
                       &model->scale.x, &model->scale.y, &model->scale.z, &collidable,
                       &model->collisionSize.x, &model->collisionSize.y, &model->collisionSize.z) != 16) goto parse_error;
            if (model->scale.x <= 0.0f || model->scale.y <= 0.0f || model->scale.z <= 0.0f) goto parse_error;
            model->collidable = collidable != 0;
            LoadModelAsset(model, directory);
            parsed.modelCount++;
            lastBox = NULL;
            continue;
        }

        if (strncmp(line, "MARKER ", 7) == 0) {
            if (parsed.markerCount >= LEVEL_MAX_MARKERS) goto parse_error;
            LevelMarker *marker = &parsed.markers[parsed.markerCount];
            if (sscanf(line, "MARKER %31s %31s %f %f %f", marker->name, marker->tag,
                       &marker->position.x, &marker->position.y, &marker->position.z) != 5) goto parse_error;
            parsed.markerCount++;
            lastBox = NULL;
            continue;
        }

        if (strncmp(line, "SLIME_VOLUME ", 13) == 0) {
            if (parsed.slimeVolumeCount >= LEVEL_MAX_SLIME_VOLUMES) goto parse_error;
            LevelSlimeVolume *volume = &parsed.slimeVolumes[parsed.slimeVolumeCount];
            char slimeName[32];
            int fields = sscanf(line, "SLIME_VOLUME %31s %31s %f %f %f %f %f %f %f %f %f %d %31s", volume->name, volume->tag,
                       &volume->center.x, &volume->center.y, &volume->center.z,
                       &volume->size.x, &volume->size.y, &volume->size.z,
                       &volume->rotation.x, &volume->rotation.y, &volume->rotation.z,
                       &volume->slimeCount, slimeName);
            if (fields != 13) {
                fields = sscanf(line, "SLIME_VOLUME %31s %31s %f %f %f %f %f %f %d %31s", volume->name, volume->tag,
                       &volume->center.x, &volume->center.y, &volume->center.z,
                       &volume->size.x, &volume->size.y, &volume->size.z, &volume->slimeCount, slimeName);
                volume->rotation = (Vector3){ 0 };
            }
            if ((fields != 13 && fields != 10) ||
                volume->size.x <= 0.0f || volume->size.y <= 0.0f || volume->size.z <= 0.0f ||
                volume->slimeCount < 1 || volume->slimeCount > 64 || !LevelSlimeTypeFromName(slimeName, &volume->slimeType)) goto parse_error;
            parsed.slimeVolumeCount++;
            lastBox = NULL;
            continue;
        }

        goto parse_error;
    }

    fclose(file);
    if (!validHeader) return false;
    LevelShutdown(level);
    *level = parsed;
    TraceLog(LOG_INFO, "LEVL: loaded %s (%d platforms, %d boxes, %d meshes, %d models, %d markers, %d slime volumes)", filePath,
             level->platformCount, level->boxCount, level->meshCount, level->modelCount, level->markerCount, level->slimeVolumeCount);
    return true;

parse_error:
    TraceLog(LOG_WARNING, "LEVL: parse error in %s near: %s", filePath, line);
    fclose(file);
    LevelShutdown(&parsed);
    return false;
}
