#include "FileSystemUtils.h"
#include <vector>
#include "Game.h"
#include "Utilities.h"
#include <string>

#include "Graphics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <physfs.h>

#include "tinyxml.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#include <winbase.h>
#define getcwd(buf, size) GetCurrentDirectory((size), (buf))
int mkdir(char* path, int mode)
{
	WCHAR utf16_path[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, utf16_path, MAX_PATH);
	return CreateDirectoryW(utf16_path, NULL);
}
#define VNEEDS_MIGRATION (mkdirResult != 0)
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__)
#include <sys/stat.h>
#include <limits.h>
#define VNEEDS_MIGRATION (mkdirResult == 0)
/* These are needed for PLATFORM_* crap */
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <spawn.h>
#define MAX_PATH PATH_MAX
#endif

char saveDir[MAX_PATH];
char levelDir[MAX_PATH];

void PLATFORM_getOSDirectory(char* output);
void PLATFORM_migrateSaveData(char* output);
void PLATFORM_copyFile(const char *oldLocation, const char *newLocation);

extern "C" {
    extern const unsigned char vce_zip[];
    extern const unsigned vce_zip_size;
}

int FILESYSTEM_init(char *argvZero)
{
	char output[MAX_PATH + 9];
	int mkdirResult;

	PHYSFS_init(argvZero);
	PHYSFS_permitSymbolicLinks(1);

	/* Determine the OS user directory */
	PLATFORM_getOSDirectory(output);

	/* Create base user directory, mount */
	mkdirResult = mkdir(output, 0777);

	/* Mount our base user directory */
	PHYSFS_mount(output, NULL, 1);
	PHYSFS_setWriteDir(output);
	if (!game.quiet) printf("Base directory: %s\n", output);

        PHYSFS_mountMemory(vce_zip, vce_zip_size, nullptr, "vce.zip", nullptr, 1);

	/* Create save directory */
	strcpy_safe(saveDir, output);
	strcat(saveDir, "saves");
	strcat(saveDir, PHYSFS_getDirSeparator());
	mkdir(saveDir, 0777);
	if (!game.quiet) printf("Save directory: %s\n", saveDir);

	/* Create level directory */
	strcpy_safe(levelDir, output);
	strcat(levelDir, "levels");
	strcat(levelDir, PHYSFS_getDirSeparator());
	mkdirResult |= mkdir(levelDir, 0777);
	if (!game.quiet) printf("Level directory: %s\n", levelDir);

	/* We didn't exist until now, migrate files! */
	if (VNEEDS_MIGRATION)
	{
		PLATFORM_migrateSaveData(output);
	}

	/* Mount the stock content last */
#ifdef __APPLE__
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
#else
	strcpy_safe(output, PHYSFS_getBaseDir());
	strcat(output, "data.zip");
#endif
	if (!PHYSFS_mount(output, NULL, 1))
	{
                if (!getcwd(output, MAX_PATH)) ;
                strcat(output, "/data.zip");
                if (!PHYSFS_mount(output, NULL, 1))
                {
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
                }
        }
#ifdef __APPLE__
        CFRelease(appUrlRef);
#endif

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

void FILESYSTEM_mount(const char *fname, Graphics& dwgfx)
{
    std::string path(PHYSFS_getRealDir(fname));
    path += pathSeparator;
    path += fname;
    if (!PHYSFS_mount(path.c_str(), NULL, 0)) {
        printf("Error mounting: %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    } else
        dwgfx.assetdir = path.c_str();
}

void FILESYSTEM_unmountassets(Graphics& dwgfx)
{
    if (dwgfx.assetdir != "")
    {
        if (!game.quiet) printf("Unmounting %s\n", dwgfx.assetdir.c_str());
        PHYSFS_unmount(dwgfx.assetdir.c_str());
        dwgfx.assetdir = "";
        dwgfx.reloadresources();
    } else if (!game.quiet) printf("Cannot unmount when no asset directory is mounted\n");
}

void FILESYSTEM_loadFileToMemory(const char *name, unsigned char **mem,
                                 size_t *len, bool addnull)
{
	PHYSFS_File *handle = PHYSFS_openRead(name);
	if (handle == NULL)
	{
		return;
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
	PHYSFS_readBytes(handle, *mem, length);
	PHYSFS_close(handle);
}

void FILESYSTEM_freeMemory(unsigned char **mem)
{
	free(*mem);
	*mem = NULL;
}

bool FILESYSTEM_saveTiXmlDocument(const char *name, TiXmlDocument *doc)
{
	/* TiXmlDocument.SaveFile doesn't account for Unicode paths, PHYSFS does */
	TiXmlPrinter printer;
	doc->Accept(&printer);
	PHYSFS_File* handle = PHYSFS_openWrite(name);
	if (handle == NULL)
	{
		return false;
	}
	PHYSFS_writeBytes(handle, printer.CStr(), printer.Size());
	PHYSFS_close(handle);
	return true;
}

bool FILESYSTEM_loadTiXmlDocument(const char *name, TiXmlDocument *doc)
{
	/* TiXmlDocument.SaveFile doesn't account for Unicode paths, PHYSFS does */
	unsigned char *mem = NULL;
	FILESYSTEM_loadFileToMemory(name, &mem, NULL, true);
	if (mem == NULL)
	{
		return false;
	}
	doc->Parse((const char*)mem, NULL, TIXML_ENCODING_UTF8);
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

void PLATFORM_getOSDirectory(char* output)
{
#ifdef _WIN32
	/* This block is here for compatibility, do not touch it! */
	WCHAR utf16_path[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, utf16_path);
	WideCharToMultiByte(CP_UTF8, 0, utf16_path, -1, output, MAX_PATH, NULL, NULL);
	strcat(output, "\\VVVVVV\\");
#else
	strlcpy(output, PHYSFS_getPrefDir("distractionware", "VVVVVV"), MAX_PATH);
#endif
}

void PLATFORM_migrateSaveData(char* output)
{
	char oldLocation[MAX_PATH];
	char newLocation[MAX_PATH];
	char oldDirectory[MAX_PATH];
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__)
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
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__)
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

#ifdef _WIN32
#include <shellapi.h>

bool FILESYSTEM_openDirectory(const char *dname) {
    ShellExecute(NULL, "open", dname, NULL, NULL, SW_SHOWMINIMIZED);
    return true;
}
#else
#ifdef __linux__
const char* open_cmd = "xdg-open";
#else
const char* open_cmd = "open";
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
