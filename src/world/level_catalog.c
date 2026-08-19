#include "level_catalog.h"

#include <raylib.h>
#include <stdio.h>
#include <string.h>

static void CopyText(char *destination, size_t destinationSize, const char *source)
{
    snprintf(destination, destinationSize, "%s", source ? source : "");
}

void LevelCatalogRefresh(LevelCatalog *catalog)
{
    *catalog = (LevelCatalog){ 0 };
    if (!DirectoryExists("levels")) return;
    FilePathList files = LoadDirectoryFilesEx("levels", ".levl", false);
    for (unsigned int i = 0; i < files.count && catalog->authoredCount < LEVEL_CATALOG_MAX_AUTHORED; ++i) {
        AuthoredLevelEntry *entry = &catalog->authored[catalog->authoredCount++];
        CopyText(entry->path, sizeof(entry->path), files.paths[i]);
        CopyText(entry->title, sizeof(entry->title), GetFileNameWithoutExt(files.paths[i]));
    }
    UnloadDirectoryFiles(files);
}
