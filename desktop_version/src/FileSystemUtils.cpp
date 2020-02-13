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

#if defined(__SWITCH__) || defined(__ANDROID__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <physfs.h>

#include "tinyxml.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <processenv.h>
#include <shellapi.h>
#include <winbase.h>
#define getcwd(buf, size) GetCurrentDirectory((size), (buf))
int mkdir(char* path, int mode)
{
	WCHAR utf16_path[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, utf16_path, MAX_PATH);
	return CreateDirectoryW(utf16_path, NULL);
}
#define VNEEDS_MIGRATION (mkdirResult != 0)
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__) || defined(__SWITCH__)
#include <sys/stat.h>
#include <limits.h>
#define VNEEDS_MIGRATION (mkdirResult == 0)
/* These are needed for PLATFORM_* crap */
#include <unistd.h>
#include <dirent.h>
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
    extern const unsigned char vce_zip[];
    extern const unsigned vce_zip_size;
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

        PHYSFS_mountMemory(vce_zip, vce_zip_size, nullptr, "vce.zip", nullptr, 0);

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
	mkdirResult = mkdir(output, 0777);

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
	mkdir(saveDir, 0777);
	if (!game.quiet) printf("Save directory: %s\n", saveDir);

        pre_fakepercent.store(10);

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

        pre_fakepercent.store(15);

	if (assetsPath) {
            strcpy_safe(output, assetsPath);
        } else {
#if defined(DATA_ZIP_PATH)
            strcpy_safe(output, DATA_ZIP_PATH);
#elif defined(__APPLE__)
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
#else
            strcpy_safe(output, PHYSFS_getBaseDir());
            strcat(output, "data.zip");
#endif
        }

        pre_fakepercent.store(20);

	if (!cached_data_zip_load(output))
	{
                if (!getcwd(output, MAX_PATH)) ;
                strcat(output, "/data.zip");
                if (!cached_data_zip_load(output))
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
	if (PHYSFS_readBytes(handle, *mem, length) == -1) {
            FILESYSTEM_freeMemory(mem);
        }
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

#ifdef _WIN32
char* FILESYSTEM_realPath(const char* rel) {
    size_t src_len = strlen(rel);
    wchar_t* srcw = (wchar_t*) malloc(src_len * 2);
    PHYSFS_utf8ToUtf16(rel, (PHYSFS_uint16*) srcw, src_len * 2);
    wchar_t* dstw = _wfullpath(nullptr, srcw, MAX_PATH);
    size_t dst_len = wcslen(dstw);
    char* dst = (char*) malloc(dst_len * 2);
    PHYSFS_utf8FromUtf16((const PHYSFS_uint16*) dstw, dst, dst_len * 2);
    return dst;
}

char* FILESYSTEM_dirname(const char* file) {
    size_t src_len = strlen(file);
    wchar_t* srcw = (wchar_t*) malloc(src_len * 2);
    PHYSFS_utf8ToUtf16(file, (PHYSFS_uint16*) srcw, src_len * 2);
    PathRemoveFileSpecW(srcw);
    wchar_t* dstw = srcw;
    size_t dst_len = wcslen(dstw);
    char* dst = (char*) malloc(dst_len * 2);
    PHYSFS_utf8FromUtf16((const PHYSFS_uint16*) dstw, dst, dst_len * 2);
    return dst;
}

char* FILESYSTEM_basename(const char* file) {
    size_t src_len = strlen(file);
    wchar_t* srcw = (wchar_t*) malloc(src_len * 2);
    PHYSFS_utf8ToUtf16(file, (PHYSFS_uint16*) srcw, src_len * 2);
    LPCWSTR dstw = PathFindFileNameW(srcw);
    size_t dst_len = wcslen(dstw);
    char* dst = (char*) malloc(dst_len * 2);
    PHYSFS_utf8FromUtf16((const PHYSFS_uint16*) dstw, dst, dst_len * 2);
    return dst;
}
#elif defined(__SWITCH__)
char* FILESYSTEM_realPath(const char* rel) {
    char* buf = (char*) malloc(MAX_PATH + 1);
    strlcpy(buf, rel, MAX_PATH + 1);
    return buf;
}

char* FILESYSTEM_dirname(const char* file) {
    return nullptr;
}

char* FILESYSTEM_basename(const char* file) {
    return nullptr;
}
#else
char* FILESYSTEM_realPath(const char* rel) {
    char* buf = (char*) malloc(MAX_PATH + 1);
    char* real = realpath(rel, buf);
    if (real) {
        return real;
    } else {
        free(buf);
        perror("realPath");
        return nullptr;
    }
}

char* FILESYSTEM_dirname(const char* file) {
    char copy[MAX_PATH];
    strcpy_safe(copy, file);
    char* dir = dirname(copy);
    size_t dir_len = strlen(dir) + 1;
    char* dir_copy = (char*) malloc(dir_len);
    bsd_strlcpy(dir_copy, dir, dir_len);
    return dir_copy;
}

char* FILESYSTEM_basename(const char* file) {
    char copy[MAX_PATH];
    strcpy_safe(copy, file);
    char* base = basename(copy);
    size_t base_len = strlen(base) + 1;
    char* base_copy = (char*) malloc(base_len);
    bsd_strlcpy(base_copy, base, base_len);
    return base_copy;
}
#endif

#ifdef _WIN32
char** FILESYSTEM_argv(int real_argc, int* argc, char* argv[]) {
    LPWSTR unparsed = GetCommandLineW();
    LPWSTR* split = CommandLineToArgvW(unparsed, argc);
    char** utf8 = (char**) malloc(*argc * sizeof(char*));
    for (int i = 0; i < *argc; ++i) {
        size_t len = wcslen(split[i]);
        utf8[i] = (char*) malloc(len * 2);
        PHYSFS_utf8FromUtf16((const PHYSFS_uint16*) split[i], utf8[i], len * 2);
    }
    LocalFree(split[0]);
    return utf8;
}
#else
char** FILESYSTEM_argv(int real_argc, int* argc, char* argv[]) {
    *argc = real_argc;
    return argv;
}
#endif
