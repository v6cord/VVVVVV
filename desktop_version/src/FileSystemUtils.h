#ifndef FILESYSTEMUTILS_H
#define FILESYSTEMUTILS_H

#include <string>
#include <vector>
#include <pugixml.hpp>
#include "Game.h"

int FILESYSTEM_init(char *argvZero);
void FILESYSTEM_deinit();

char *FILESYSTEM_getUserSaveDirectory();
char *FILESYSTEM_getUserLevelDirectory();

bool FILESYSTEM_directoryExists(const char *fname);
void FILESYSTEM_mount(const char *fname);
void FILESYSTEM_loadFileToMemory(const char *name, unsigned char **mem, size_t *len);
void FILESYSTEM_freeMemory(unsigned char **mem);
bool FILESYSTEM_saveXmlDocument(const char *name, pugi::xml_document *doc);
bool FILESYSTEM_loadXmlDocument(const char *name, pugi::xml_document *doc);

growing_vector<std::string> FILESYSTEM_getLevelDirFileNames();

#endif /* FILESYSTEMUTILS_H */
