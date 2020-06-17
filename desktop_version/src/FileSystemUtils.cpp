#include "FileSystemUtils.h"
#include <vector>
#include "Game.h"
#include "Utilities.h"
#include "preloader.h"
#include <string>

#include "Graphics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iterator>
#include <algorithm>
#include <iostream>

#include <SDL.h>
#include <physfs.h>

#include "tinyxml2.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

/* These are needed for PLATFORM_* crap */
#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <processenv.h>
#include <shellapi.h>
#include <winbase.h>
#define getcwd(buf, size) GetCurrentDirectory((size), (buf))
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__) || defined(__DragonFly__) || defined(__SWITCH__)
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <spawn.h>
#include <libgen.h>
#define MAX_PATH PATH_MAX
#endif

char saveDir[MAX_PATH];
char levelDir[MAX_PATH];

void PLATFORM_getOSDirectory(char* output);
void PLATFORM_migrateSaveData(char* output);
void PLATFORM_copyFile(const char *oldLocation, const char *newLocation);

extern "C" {
#ifdef LD_VCE_ZIP
    extern const char _binary_vce_zip_start;
    extern const char _binary_vce_zip_end;
#else
    extern const unsigned char vce_zip[];
    extern const unsigned vce_zip_size;
#endif
}

static bool cached_data_zip_load(const char* path) {
#ifdef __SWITCH__
    FILE* file = fopen(path, "rb");
    if (file == nullptr) return false;
    if (fseek(file, 0L, SEEK_END)) {
        fclose(file);
        return false;
    }
    long size = ftell(file);
    if (size == -1) {
        fclose(file);
        return false;
    }
    if (fseek(file, 0L, SEEK_SET)) {
        fclose(file);
        return false;
    }
    unsigned char* buf = (unsigned char*) malloc(size);
    size_t read = fread(buf, 1, size, file);
    if (read != (long unsigned int) size && ferror(file)) {
        fclose(file);
        free(buf);
        return false;
    }
    fclose(file);

    if (!PHYSFS_mountMemory(buf, read, nullptr, "data.zip", NULL, 1)) {
        free(buf);
        return false;
    }
    return true;
#elif defined(__ANDROID__)
    SDL_RWops* file = SDL_RWFromFile("data.zip", "rb");
    if (file == nullptr) return false;
    long size = file->size(file);
    if (size == -1) {
        file->close(file);
        return false;
    }
    unsigned char* buf = (unsigned char*) malloc(size);
    size_t read = file->read(file, buf, 1, size);
    if (read == 0) {
        file->close(file);
        free(buf);
        return false;
    }
    file->close(file);

    if (!PHYSFS_mountMemory(buf, read, nullptr, "data.zip", NULL, 1)) {
        free(buf);
        return false;
    }
    return true;
#else
    return PHYSFS_mount(path, NULL, 1);
#endif
}

int FILESYSTEM_initCore(char *argvZero, char *baseDir, char *assetsPath)
{
	char output[MAX_PATH + 9];
	const char* pathSep = PHYSFS_getDirSeparator();

	PHYSFS_init(argvZero);
	PHYSFS_permitSymbolicLinks(1);

#ifdef LD_VCE_ZIP
        PHYSFS_mountMemory(&_binary_vce_zip_start, &_binary_vce_zip_end - &_binary_vce_zip_start, nullptr, "vce.zip", nullptr, 0);
#else
        PHYSFS_mountMemory(vce_zip, vce_zip_size, nullptr, "vce.zip", nullptr, 0);
#endif

	/* Determine the OS user directory */
	if (baseDir && strlen(baseDir) > 0)
	{
		strcpy(output, baseDir);

		/* We later append to this path and assume it ends in a slash */
		if (strcmp(std::string(1, output[strlen(output) - 1]).c_str(), pathSep) != 0)
		{
			strcat(output, pathSep);
		}
	}
	else
	{
		PLATFORM_getOSDirectory(output);
	}
	PHYSFS_mount(output, NULL, 0);

        return 1;
}

int FILESYSTEM_init(char *argvZero, char *baseDir, char *assetsPath)
{
	char output[MAX_PATH + 9];
	int mkdirResult;
	const char* pathSep = PHYSFS_getDirSeparator();

	/* Determine the OS user directory */
	if (baseDir && strlen(baseDir) > 0)
	{
		strcpy(output, baseDir);

		/* We later append to this path and assume it ends in a slash */
		if (strcmp(std::string(1, output[strlen(output) - 1]).c_str(), pathSep) != 0)
		{
			strcat(output, pathSep);
		}
	}
	else
	{
		PLATFORM_getOSDirectory(output);
	}

	/* Create base user directory, mount */
	mkdirResult = PHYSFS_mkdir(output);

	/* Mount our base user directory */
        PHYSFS_unmount(output);
	PHYSFS_mount(output, NULL, 0);
	PHYSFS_setWriteDir(output);
	if (!game.quiet) printf("Base directory: %s\n", output);

        pre_fakepercent.store(5);

	/* Create save directory */
	strcpy_safe(saveDir, output);
	strcat(saveDir, "saves");
	strcat(saveDir, PHYSFS_getDirSeparator());
	PHYSFS_mkdir(saveDir);
	if (!game.quiet) printf("Save directory: %s\n", saveDir);

        pre_fakepercent.store(10);

	/* Create level directory */
	strcpy_safe(levelDir, output);
	strcat(levelDir, "levels");
	strcat(levelDir, PHYSFS_getDirSeparator());
	mkdirResult |= PHYSFS_mkdir(levelDir);
	if (!game.quiet) printf("Level directory: %s\n", levelDir);

	/* We didn't exist until now, migrate files! */
	if (mkdirResult != 0)
	{
		PLATFORM_migrateSaveData(output);
	}

	/* Mount the stock content last */

        pre_fakepercent.store(15);

	if (assetsPath) {
            strcpy_safe(output, assetsPath);
        } else {
#if defined(__APPLE__)
            CFURLRef appUrlRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("data.zip"), NULL, NULL);
            if (!appUrlRef) {
                SDL_ShowSimpleMessageBox(
                        SDL_MESSAGEBOX_ERROR,
                        "Couldn't find data.zip in .app!",
                        "Please place data.zip in Contents/Resources\n"
                        "inside VVVVVV-CE.app.",
                        NULL
                        );
                return 0;
            }
            if (!CFURLGetFileSystemRepresentation(appUrlRef, true, (uint8_t*) output, MAX_PATH)) {
                SDL_ShowSimpleMessageBox(
                        SDL_MESSAGEBOX_ERROR,
                        "Couldn't get data.zip path!",
                        "Please report this error.",
                        NULL
                        );
                return 0;
            }
            CFRelease(appUrlRef);
#elif defined(DATA_ZIP_PATH)
            strcpy_safe(output, DATA_ZIP_PATH);
#else
            strcpy_safe(output, PHYSFS_getBaseDir());
            strcat(output, "data.zip");
#endif
        }

        pre_fakepercent.store(20);

	if (!cached_data_zip_load(output))
	{
#ifndef __APPLE__
                if (!getcwd(output, MAX_PATH)) ;
                strcat(output, "/data.zip");
                if (!cached_data_zip_load(output))
                {
#endif
                        puts("Error: data.zip missing!");
                        puts("You do not have data.zip!");
                        puts("Grab it from your purchased copy of the game,");
                        puts("or get it from the free Make and Play Edition.");

                        std::string message = "You do not have data.zip at ";
                        message += output;
                        message += "!\n\nGrab it from your purchased copy of the game,"
                                    "\nor get it from the free Make and Play Edition.";
                        SDL_ShowSimpleMessageBox(
                                SDL_MESSAGEBOX_ERROR,
                                "data.zip missing!",
                                message.c_str(),
                                NULL
                        );
                        return 0;
#ifndef __APPLE__
                }
#endif
        }

        pre_fakepercent.store(45);

	strcpy_safe(output, PHYSFS_getBaseDir());
	strcpy_safe(output, "gamecontrollerdb.txt");
	if (SDL_GameControllerAddMappingsFromFile(output) < 0 && !game.quiet)
	{
		printf("gamecontrollerdb.txt not found!\n");
	}

	return 1;
}

void FILESYSTEM_deinit()
{
	PHYSFS_deinit();
}

char *FILESYSTEM_getUserSaveDirectory()
{
	return saveDir;
}

char *FILESYSTEM_getUserLevelDirectory()
{
	return levelDir;
}

bool FILESYSTEM_directoryExists(const char *fname)
{
    return PHYSFS_exists(fname);
}

const char pathSeparator =
#ifdef _WIN32
                            '\\';
#else
                            '/';
#endif

void FILESYSTEM_mount(const char *fname)
{
    std::string path(PHYSFS_getRealDir(fname));
    path += pathSeparator;
    path += fname;
    if (!PHYSFS_mount(path.c_str(), NULL, 0)) {
        printf("Error mounting: %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    } else
        graphics.assetdir = path.c_str();
}

void FILESYSTEM_unmountassets()
{
    if (graphics.assetdir != "")
    {
        if (!game.quiet) printf("Unmounting %s\n", graphics.assetdir.c_str());
        PHYSFS_unmount(graphics.assetdir.c_str());
        graphics.assetdir = "";
        graphics.reloadresources();
    } else if (!game.quiet) printf("Cannot unmount when no asset directory is mounted\n");
}

bool FILESYSTEM_loadFileToMemory(const char *name, unsigned char **mem,
                                 size_t *len, bool addnull)
{
        if (strcmp(name, "levels/special/stdin.vvvvvv") == 0) {
            // this isn't *technically* necessary when piping directly from a file, but checking for that is annoying
            static std::vector<char> STDIN_BUFFER;
            static bool STDIN_LOADED = false;
            if (!STDIN_LOADED) {
                std::istreambuf_iterator<char> begin(std::cin), end;
                STDIN_BUFFER.assign(begin, end);
                STDIN_BUFFER.push_back(0); // there's no observable change in behavior if addnull is always true, but not vice versa
                STDIN_LOADED = true;
            }

            auto length = STDIN_BUFFER.size() - 1;
            if (len != NULL) {
                *len = length;
            }

            ++length;
            *mem = static_cast<unsigned char*>(malloc(length)); // STDIN_BUFFER.data() causes double-free
            std::copy(STDIN_BUFFER.begin(), STDIN_BUFFER.end(), reinterpret_cast<char*>(*mem));
            return true;
        }

	PHYSFS_File *handle = PHYSFS_openRead(name);
	if (handle == NULL)
	{
		return false;
	}
	PHYSFS_uint32 length = PHYSFS_fileLength(handle);
	if (len != NULL)
	{
		*len = length;
	}
	if (addnull)
	{
		*mem = (unsigned char *) malloc(length + 1);
		(*mem)[length] = 0;
	}
	else
	{
		*mem = (unsigned char*) malloc(length);
	}
	if (PHYSFS_readBytes(handle, *mem, length) == -1) {
            FILESYSTEM_freeMemory(mem);
        }
	PHYSFS_close(handle);
    return true;
}

void FILESYSTEM_freeMemory(unsigned char **mem)
{
	free(*mem);
	*mem = NULL;
}

bool FILESYSTEM_saveTiXml2Document(const char *name, tinyxml2::XMLDocument& doc)
{
	/* XMLDocument.SaveFile doesn't account for Unicode paths, PHYSFS does */
	tinyxml2::XMLPrinter printer;
	doc.Print(&printer);
	PHYSFS_File* handle = PHYSFS_openWrite(name);
	if (handle == NULL)
	{
		return false;
	}
	PHYSFS_writeBytes(handle, printer.CStr(), printer.CStrSize() - 1); // subtract one because CStrSize includes terminating null
	PHYSFS_close(handle);
	return true;
}

bool FILESYSTEM_loadTiXml2Document(const char *name, tinyxml2::XMLDocument& doc)
{
	/* XMLDocument.LoadFile doesn't account for Unicode paths, PHYSFS does */
	unsigned char *mem = NULL;
	FILESYSTEM_loadFileToMemory(name, &mem, NULL, true);
	if (mem == NULL)
	{
		return false;
	}
	doc.Parse((const char*) mem);
	FILESYSTEM_freeMemory(&mem);
	return true;
}

growing_vector<std::string> FILESYSTEM_getLevelDirFileNames()
{
	growing_vector<std::string> list;
	char **fileList = PHYSFS_enumerateFiles("/levels");
	char **i;
	std::string builtLocation;

	for (i = fileList; *i != NULL; i++)
	{
		if (strcmp(*i, "data") == 0)
		{
			continue; /* FIXME: lolwut -flibit */
		}
		builtLocation = "levels/";
		builtLocation += *i;
		list.push_back(builtLocation);
	}

	PHYSFS_freeList(fileList);

	return list;
}

std::vector<std::string> FILESYSTEM_getGraphicsDirFileNames()
{
	std::vector<std::string> list;
	char **fileList = PHYSFS_enumerateFiles("/graphics");
	std::string builtLocation;

	for (char **i = fileList; *i != NULL; i++)
	{
		if (strcmp(*i, "data") == 0)
		{
			continue;
		}
		builtLocation = "graphics/";
		builtLocation += *i;
		list.push_back(builtLocation);
	}

	PHYSFS_freeList(fileList);
	return list;
}

void PLATFORM_getOSDirectory(char* output)
{
#ifdef _WIN32
	/* This block is here for compatibility, do not touch it! */
	WCHAR utf16_path[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, utf16_path);
	WideCharToMultiByte(CP_UTF8, 0, utf16_path, -1, output, MAX_PATH, NULL, NULL);
	strcat(output, "\\VVVVVV\\");
#elif defined(__SWITCH__)
	bsd_strlcpy(output, "sdmc:/switch/VVVVVV/", MAX_PATH);
#elif defined(__ANDROID__)
        bsd_strlcpy(output, SDL_AndroidGetExternalStoragePath(), MAX_PATH - 1);
        strcat(output, "/");
#else
	bsd_strlcpy(output, PHYSFS_getPrefDir("distractionware", "VVVVVV"), MAX_PATH);
#endif
}

void PLATFORM_migrateSaveData(char* output)
{
#if !defined(__SWITCH__)
	char oldLocation[MAX_PATH];
	char newLocation[MAX_PATH];
	char oldDirectory[MAX_PATH];
#endif

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__) || defined(__DragonFly__)
	DIR *dir = NULL;
	struct dirent *de = NULL;
	DIR *subDir = NULL;
	struct dirent *subDe = NULL;
	char subDirLocation[MAX_PATH];
	const char *homeDir = getenv("HOME");
	if (homeDir == NULL)
	{
		/* Uhh, I don't want to get near this. -flibit */
		return;
	}
	strcpy_safe(oldDirectory, homeDir);
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__) || defined(__DragonFly__)
	strcat(oldDirectory, "/.vvvvvv/");
#elif defined(__APPLE__)
	strcat(oldDirectory, "/Documents/VVVVVV/");
#endif
	dir = opendir(oldDirectory);
	if (!dir)
	{
		printf("Could not find directory %s\n", oldDirectory);
		return;
	}

	printf("Migrating old savedata to new location...\n");
	for (de = readdir(dir); de != NULL; de = readdir(dir))
	{
		if (	strcmp(de->d_name, "..") == 0 ||
			strcmp(de->d_name, ".") == 0	)
		{
			continue;
		}
		#define COPY_SAVEFILE(name) \
			else if (strcmp(de->d_name, name) == 0) \
			{ \
				strcpy_safe(oldLocation, oldDirectory); \
				strcat(oldLocation, name); \
				strcpy_safe(newLocation, output); \
				strcat(newLocation, "saves/"); \
				strcat(newLocation, name); \
				PLATFORM_copyFile(oldLocation, newLocation); \
			}
		COPY_SAVEFILE("unlock.vvv")
		COPY_SAVEFILE("tsave.vvv")
		COPY_SAVEFILE("qsave.vvv")
		#undef COPY_SAVEFILE
		else if (strstr(de->d_name, ".vvvvvv.vvv") != NULL)
		{
			strcpy_safe(oldLocation, oldDirectory);
			strcat(oldLocation, de->d_name);
			strcpy_safe(newLocation, output);
			strcat(newLocation, "saves/");
			strcat(newLocation, de->d_name);
			PLATFORM_copyFile(oldLocation, newLocation);
		}
		else if (strstr(de->d_name, ".vvvvvv") != NULL)
		{
			strcpy_safe(oldLocation, oldDirectory);
			strcat(oldLocation, de->d_name);
			strcpy_safe(newLocation, output);
			strcat(newLocation, "levels/");
			strcat(newLocation, de->d_name);
			PLATFORM_copyFile(oldLocation, newLocation);
		}
		else if (strcmp(de->d_name, "Saves") == 0)
		{
			strcpy_safe(subDirLocation, oldDirectory);
			strcat(subDirLocation, "Saves/");
			subDir = opendir(subDirLocation);
			if (!subDir)
			{
				printf("Could not open Saves/ subdir!\n");
				continue;
			}
			for (
				subDe = readdir(subDir);
				subDe != NULL;
				subDe = readdir(subDir)
			) {
				#define COPY_SAVEFILE(name) \
					(strcmp(subDe->d_name, name) == 0) \
					{ \
						strcpy_safe(oldLocation, subDirLocation); \
						strcat(oldLocation, name); \
						strcpy_safe(newLocation, output); \
						strcat(newLocation, "saves/"); \
						strcat(newLocation, name); \
						PLATFORM_copyFile(oldLocation, newLocation); \
					}
				if COPY_SAVEFILE("unlock.vvv")
				else if COPY_SAVEFILE("tsave.vvv")
				else if COPY_SAVEFILE("qsave.vvv")
				#undef COPY_SAVEFILE
			}
		}
	}
#elif defined(_WIN32)
	WIN32_FIND_DATA findHandle;
	HANDLE hFind = NULL;
	char fileSearch[MAX_PATH + 9];

	/* Same place, different layout. */
	strcpy_safe(oldDirectory, output);

	/* In theory we don't need to worry about this, thanks case insensitivity!
	sprintf(fileSearch, "%s\\Saves\\*.vvv", oldDirectory);
	hFind = FindFirstFile(fileSearch, &findHandle);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("Could not find directory %s\\Saves\\\n", oldDirectory);
	}
	else do
	{
		if ((findHandle.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			#define COPY_SAVEFILE(name) \
				(strcmp(findHandle.cFileName, name) == 0) \
				{ \
					strcpy_safe(oldLocation, oldDirectory); \
					strcat(oldLocation, "Saves\\"); \
					strcat(oldLocation, name); \
					strcpy_safe(newLocation, output); \
					strcat(newLocation, "saves\\"); \
					strcat(newLocation, name); \
					PLATFORM_copyFile(oldLocation, newLocation); \
				}
			if COPY_SAVEFILE("unlock.vvv")
			else if COPY_SAVEFILE("tsave.vvv")
			else if COPY_SAVEFILE("qsave.vvv")
			#undef COPY_SAVEFILE
		}
	} while (FindNextFile(hFind, &findHandle));
	*/

	sprintf(fileSearch, "%s\\*.vvvvvv", oldDirectory);
	hFind = FindFirstFile(fileSearch, &findHandle);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("Could not find directory %s\n", oldDirectory);
	}
	else do
	{
		if ((findHandle.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			strcpy_safe(oldLocation, oldDirectory);
			strcat(oldLocation, findHandle.cFileName);
			strcpy_safe(newLocation, output);
			strcat(newLocation, "levels\\");
			strcat(newLocation, findHandle.cFileName);
			PLATFORM_copyFile(oldLocation, newLocation);
		}
	} while (FindNextFile(hFind, &findHandle));
#elif defined(__SWITCH__)
	/* No Migration needed. */
#else
#error See PLATFORM_migrateSaveData
#endif
}

void PLATFORM_copyFile(const char *oldLocation, const char *newLocation)
{
	char *data;
	long int length;

	/* Read data */
	FILE *file = fopen(oldLocation, "rb");
	if (!file)
	{
		printf("Cannot open/copy %s\n", oldLocation);
		return;
	}
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);
	data = (char*) malloc(length);
	if (fread(data, 1, length, file) <= 0) {
            printf("it broke!!!\n");
            exit(1);
        }
	fclose(file);

	/* Write data */
	file = fopen(newLocation, "wb");
	if (!file)
	{
		printf("Could not write to %s\n", newLocation);
		free(data);
		return;
	}
	fwrite(data, 1, length, file);
	fclose(file);
	free(data);

	/* WTF did we just do */
	printf("Copied:\n\tOld: %s\n\tNew: %s\n", oldLocation, newLocation);
}

bool FILESYSTEM_openDirectoryEnabled()
{
	/* This is just a check to see if we're on a desktop or tenfoot setup.
	 * If you're working on a tenfoot-only build, add a def that always
	 * returns false!
	 */
#if defined(__SWITCH) || defined(__ANDROID__)
	return false;
#else
	return !SDL_GetHintBoolean("SteamTenfoot", SDL_FALSE);
#endif
}

#ifdef _WIN32
#include <shellapi.h>
bool FILESYSTEM_openDirectory(const char *dname) {
    ShellExecute(NULL, "open", dname, NULL, NULL, SW_SHOWMINIMIZED);
    return true;
}
#elif defined(__SWITCH__) || defined(__ANDROID__)
bool FILESYSTEM_openDirectory(const char *dname) {
    return false;
}
#else
#if defined(__APPLE__) || defined(__HAIKU__)
const char* open_cmd = "open";
#else
const char* open_cmd = "xdg-open";
#endif

extern "C" char** environ;

bool FILESYSTEM_openDirectory(const char *dname) {
    pid_t child;
    // This const_cast is legal (ctrl-f "The statement" at https://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html)
    char* argv[3] = {const_cast<char*>(open_cmd), const_cast<char*>(dname), nullptr};
    posix_spawnp(
            &child, // pid
            open_cmd, // file
            nullptr, // file_actions
            nullptr, // attrp
            argv, // argv
            environ // envp
    );
    int status = 0;
    waitpid(child, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
#endif

bool FILESYSTEM_delete(const char *name)
{
    return PHYSFS_delete(name) != 0;
}
