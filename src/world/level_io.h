#ifndef SLIMEWORKS_LEVEL_IO_H
#define SLIMEWORKS_LEVEL_IO_H

#include "level.h"

/* Parses the stable text format produced by Slimeworks Level Editor. */
bool LevelLoadFromFile(Level *level, const char *filePath);

#endif
