#include "SDL.h"
#include "physfs.h"
#include "FileSystemUtils.h"
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#define VNEEDS_MIGRATION (mkdirResult != 0)
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/stat.h>
#include <limits.h>
#define MAX_PATH PATH_MAX
#define VNEEDS_MIGRATION (mkdirResult == 0)
/* These are needed for PLATFORM_* crap */
#include <unistd.h>
#include <dirent.h>
#endif

namespace stdfs = std::filesystem;

//Global instance
static FSUtils* s_pFSUtilsInstance = nullptr;

//Old code

void PLATFORM_copyFile(const char* oldLocation, const char* newLocation)
{
	char* data;
	long int length;

	/* Read data */
	FILE* file = fopen(oldLocation, "rb");
	if (!file)
	{
		printf("Cannot open/copy %s\n", oldLocation);
		return;
	}
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);
	data = (char*)malloc(length);
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

void PLATFORM_migrateSaveData(char* output)
{
	char oldLocation[MAX_PATH];
	char newLocation[MAX_PATH];
	char oldDirectory[MAX_PATH];
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	DIR* dir = NULL;
	struct dirent* de = NULL;
	DIR* subDir = NULL;
	struct dirent* subDe = NULL;
	char subDirLocation[MAX_PATH];
	const char* homeDir = getenv("HOME");
	if (homeDir == NULL)
	{
		/* Uhh, I don't want to get near this. -flibit */
		return;
	}
	strcpy(oldDirectory, homeDir);
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
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
		if (strcmp(de->d_name, "..") == 0 ||
			strcmp(de->d_name, ".") == 0)
		{
			continue;
		}
#define COPY_SAVEFILE(name) \
			else if (strcmp(de->d_name, name) == 0) \
			{ \
				strcpy(oldLocation, oldDirectory); \
				strcat(oldLocation, name); \
				strcpy(newLocation, output); \
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
			strcpy(oldLocation, oldDirectory);
			strcat(oldLocation, de->d_name);
			strcpy(newLocation, output);
			strcat(newLocation, "saves/");
			strcat(newLocation, de->d_name);
			PLATFORM_copyFile(oldLocation, newLocation);
		}
			else if (strstr(de->d_name, ".vvvvvv") != NULL)
		{
			strcpy(oldLocation, oldDirectory);
			strcat(oldLocation, de->d_name);
			strcpy(newLocation, output);
			strcat(newLocation, "levels/");
			strcat(newLocation, de->d_name);
			PLATFORM_copyFile(oldLocation, newLocation);
		}
			else if (strcmp(de->d_name, "Saves") == 0)
		{
			strcpy(subDirLocation, oldDirectory);
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
						strcpy(oldLocation, subDirLocation); \
						strcat(oldLocation, name); \
						strcpy(newLocation, output); \
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
	strcpy(oldDirectory, output);

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
					strcpy(oldLocation, oldDirectory); \
					strcat(oldLocation, "Saves\\"); \
					strcat(oldLocation, name); \
					strcpy(newLocation, output); \
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
			strcpy(oldLocation, oldDirectory);
			strcat(oldLocation, findHandle.cFileName);
			strcpy(newLocation, output);
			strcat(newLocation, "levels\\");
			strcat(newLocation, findHandle.cFileName);
			PLATFORM_copyFile(oldLocation, newLocation);
		}
	} while (FindNextFile(hFind, &findHandle));
#else
#error See PLATFORM_migrateSaveData
#endif
}

//Helpers

bool createDirectory(stdfs::path const& path)
{
	if (!stdfs::exists(path))
		return stdfs::create_directory(path);

	return true;
}

//TODO: Error checking
bool getOSDirectory(stdfs::path& out)
{
	out.clear();

#ifdef _WIN32
	CHAR path[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);
	out = path;
	out += "\\VVVVVV\\";
	return true;
#else
	out = PHYSFS_getPrefDir("distractionware", "VVVVVV");
	return true;
#endif
}

//FSUtils

//TODO: Error checking

/*
int FILESYSTEM_init(char *argvZero)
{
	//We didn't exist until now, migrate files!
if (VNEEDS_MIGRATION)
{
	PLATFORM_migrateSaveData(output);
}
}
*/

FSUtils::FSUtils(std::string const& argvZero)
{
	int mkdirResult = 0;

	//Init PHYSFS
	PHYSFS_init(argvZero.c_str());
	PHYSFS_permitSymbolicLinks(true);

	//Determine OS user directory
	getOSDirectory(m_pBaseDir);

	//Create base directory
	createDirectory(m_pBaseDir);

	//Mount directory
	PHYSFS_mount(m_pBaseDir.string().c_str(), nullptr, true);
	PHYSFS_setWriteDir(m_pBaseDir.string().c_str());
	std::clog << "Base directory: " << m_pBaseDir << std::endl;

	//Create save directory
	m_pSaveDir = m_pBaseDir.string() + "saves";
	m_pSaveDir = m_pSaveDir.string() + PHYSFS_getDirSeparator();
	createDirectory(m_pSaveDir);
	std::clog << "Save directory: " << m_pSaveDir << std::endl;

	//Create level directory
	m_pLevelDir = m_pBaseDir.string() + "levels";
	m_pLevelDir = m_pLevelDir.string() + PHYSFS_getDirSeparator();
	createDirectory(m_pLevelDir);
	std::clog << "Level directory: " << m_pLevelDir << std::endl;

	//Migrate files
	//TODO: rework it
	if (VNEEDS_MIGRATION)
	{
		PLATFORM_migrateSaveData(const_cast<char*>(m_pLevelDir.string().c_str()));
	}

	//Mount data.zip
	m_pDataDir = PHYSFS_getBaseDir() + std::string("data.zip");
	if (!PHYSFS_mount(m_pDataDir.string().c_str(), nullptr, true))
	{
		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"data.zip missing!",
			"You do not have data.zip!"
			"\n\nGrab it from your purchased copy of the game,"
			"\nor get it from the free Make and Play Edition.",
			nullptr);
		throw std::runtime_error("Data.zip not found.");
	}
}

FSUtils::~FSUtils()
{
	PHYSFS_deinit();
}

FSUtils* FSUtils::create(std::string const& argvZero)
{
	auto p = new FSUtils(argvZero);

	if (p)
	{
		s_pFSUtilsInstance = p;
		return p;
	}

	return nullptr;
}

void FSUtils::destroy()
{
	if (s_pFSUtilsInstance)
	{
		delete s_pFSUtilsInstance;
		s_pFSUtilsInstance = nullptr;
	}
}

FSUtils* FSUtils::getInstance()
{
	return s_pFSUtilsInstance;
}

auto FSUtils::levelNames()
-> std::vector<std::string>
{
	std::vector<std::string> vec;

	auto fileList = PHYSFS_enumerateFiles("/levels");

	if (fileList)
	{
		for (auto i = fileList; *i; ++i)
		{
			if (*i == std::string("data"))
				continue; //FIXME: lolwut -flibit

			vec.push_back(std::string("levels/") + *i);
		}
	}

	PHYSFS_freeList(fileList);
	return vec;
}

bool FSUtils::loadFile(
	std::string const& name,
	std::vector<uint8_t>& buffer)
{
	auto handle = PHYSFS_openRead(name.c_str());
	
	if (!handle)
		return false;

	auto size = PHYSFS_fileLength(handle);

	if (!size)
		return false;

	buffer.clear();

	auto p = new uint8_t[size];
	PHYSFS_readBytes(handle, reinterpret_cast<void*>(p), size);

	buffer = std::vector<uint8_t>(p, p + size);

	delete[] p;
	PHYSFS_close(handle);
	return true;
}

bool FSUtils::loadXml(
	std::string const& name,
	TiXmlDocument& doc)
{
	std::vector<uint8_t> buffer;

	if (this->loadFile(name, buffer))
	{
		doc.Parse(
			reinterpret_cast<char*>(buffer.data()),
				nullptr, TIXML_ENCODING_UTF8);
		return true;
	}

        return false;
}

bool FSUtils::saveXml(
	std::string const& name,
	TiXmlDocument const& doc)
{
	TiXmlPrinter printer;
	doc.Accept(&printer);

	auto handle = PHYSFS_openWrite(name.c_str());

	if (handle)
	{
		PHYSFS_writeBytes(handle, printer.CStr(), printer.Size());
		PHYSFS_close(handle);
		return true;
	}

	return false;
}
