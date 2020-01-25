#ifndef FILESYSTEMUTILS_H
#define FILESYSTEMUTILS_H

#include <string>
#include <vector>
#include "Game.h"

#include "tinyxml.h"

#include "tinyxml.h"

int FILESYSTEM_init(char *argvZero);
void FILESYSTEM_deinit();

char *FILESYSTEM_getUserSaveDirectory();
char *FILESYSTEM_getUserLevelDirectory();

bool FILESYSTEM_directoryExists(const char *fname);
void FILESYSTEM_mount(const char *fname, Graphics& dwgfx);
void FILESYSTEM_unmountassets(Graphics& dwgfx);
void FILESYSTEM_loadFileToMemory(const char *name, unsigned char **mem,
                                 size_t *len, bool addnull = false);
void FILESYSTEM_freeMemory(unsigned char **mem);
bool FILESYSTEM_saveTiXmlDocument(const char *name, TiXmlDocument *doc);
bool FILESYSTEM_loadTiXmlDocument(const char *name, TiXmlDocument *doc);

growing_vector<std::string> FILESYSTEM_getLevelDirFileNames();

bool FILESYSTEM_openDirectory(const char *dname);

#endif /* FILESYSTEMUTILS_H */
