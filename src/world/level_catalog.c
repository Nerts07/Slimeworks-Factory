#include "level_catalog.h"

#include <raylib.h>
#include <stdio.h>
#include <string.h>

static void CopyText(char *destination, size_t destinationSize, const char *source)
{
    snprintf(destination, destinationSize, "%s", source ? source : "");
}

static void AddCatalogEntry(LevelCatalog *catalog, const char *path, const char *title)
{
    if (catalog->authoredCount >= LEVEL_CATALOG_MAX_AUTHORED || !FileExists(path)) return;
    AuthoredLevelEntry *entry = &catalog->authored[catalog->authoredCount++];
    CopyText(entry->path, sizeof(entry->path), path);
    CopyText(entry->title, sizeof(entry->title), title);
}

static void AddDirectoryEntries(LevelCatalog *catalog, const char *directory, const char *label)
{
    if (!DirectoryExists(directory)) return;
    FilePathList files = LoadDirectoryFilesEx(directory, ".levl", false);
    for (unsigned int i = 0; i < files.count && catalog->authoredCount < LEVEL_CATALOG_MAX_AUTHORED; ++i) {
        char title[LEVEL_CATALOG_TITLE_LENGTH];
        snprintf(title, sizeof(title), "%s — %s", label, GetFileNameWithoutExt(files.paths[i]));
        AddCatalogEntry(catalog, files.paths[i], title);
    }
    UnloadDirectoryFiles(files);
}

void LevelCatalogRefresh(LevelCatalog *catalog)
{
    *catalog = (LevelCatalog){ 0 };
    /* The editor draft is the current user-authored level and must appear first. */
    AddCatalogEntry(catalog, "../Slimeworks-Level-Editor/level.levl", "Editor Draft");
    /* Support the older workspace folder name as well. */
    AddDirectoryEntries(catalog, "../Slimeworks Factory/levels", "Workspace Export");
    AddDirectoryEntries(catalog, "levels", "Game Levels");
}
