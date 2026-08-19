#ifndef SLIMEWORKS_LEVEL_CATALOG_H
#define SLIMEWORKS_LEVEL_CATALOG_H

#include <stdbool.h>

#define LEVEL_CATALOG_MAX_AUTHORED 32
#define LEVEL_CATALOG_TITLE_LENGTH 64
#define LEVEL_CATALOG_PATH_LENGTH 512

typedef struct AuthoredLevelEntry {
    char title[LEVEL_CATALOG_TITLE_LENGTH];
    char path[LEVEL_CATALOG_PATH_LENGTH];
} AuthoredLevelEntry;

typedef struct LevelCatalog {
    AuthoredLevelEntry authored[LEVEL_CATALOG_MAX_AUTHORED];
    int authoredCount;
} LevelCatalog;

/* Finds .levl files in levels/. Call again after exporting a new map. */
void LevelCatalogRefresh(LevelCatalog *catalog);

#endif
