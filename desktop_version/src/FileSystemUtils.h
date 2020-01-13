#ifndef FILESYSTEMUTILS_H
#define FILESYSTEMUTILS_H

#include "tinyxml.h"
#include <string>
#include <vector>
#include <filesystem>

class FSUtils
{
protected:
	std::filesystem::path m_pBaseDir;
	std::filesystem::path m_pSaveDir;
	std::filesystem::path m_pLevelDir;
	std::filesystem::path m_pDataDir;

	FSUtils(std::string const& argvZero);
	~FSUtils();
public:
	static FSUtils* create(std::string const& argvZero);
	static void destroy();
	static FSUtils* getInstance();

	std::filesystem::path const& saveDirectory() { return m_pSaveDir; }
	std::filesystem::path const& levelDirectory() { return m_pLevelDir; }

	std::vector<std::string> levelNames();

	bool loadFile(
		std::string const& name,
		std::vector<uint8_t>& buffer);

	bool loadXml(
		std::string const& name,
		TiXmlDocument& doc);

	bool saveXml(
		std::string const& name,
		TiXmlDocument const& doc);
};

#endif /* FILESYSTEMUTILS_H */
