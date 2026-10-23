#include "mod-loader.h"
#include "stub-functions.h"
#include "helpers.h"
#include "logger.h"
#include <INIReader.h>

void ModLoader::init(HMODULE module) {
    char buffer[4096];
	buffer[0] = 0;
	GetModuleFileNameA(module, buffer, sizeof(buffer));
    std::filesystem::path filepath{ buffer };
	ResolveStubMethods(LoadSystemLibrary(filepath.filename().string().c_str()));

    modFolder = filepath.parent_path() / "mods";

	if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
		AllocConsole();
    	SetConsoleOutputCP(CP_UTF8);
	}
	freopen("CONOUT$", "w", stdout);

	std::filesystem::path mainIniFilepath{ modFolder };
	mainIniFilepath /= "mods.ini";

	DWORD dllDirSize = GetDllDirectoryA(0, nullptr)+1;
	char* dllDir = new char[dllDirSize];
	GetDllDirectoryA(dllDirSize, dllDir);

	DWORD curDirSize = GetCurrentDirectoryA(0, nullptr)+1;
	char* curDir = new char[curDirSize];
	GetCurrentDirectoryA(curDirSize, curDir);

	INIReader mainIni(mainIniFilepath.string());

	int modCount = mainIni.GetInteger("Main", "ActiveModCount", 0);

	for (int i = 0; i < modCount; i++) {
		char key[16];
		snprintf(key, sizeof(key), "ActiveMod%u", i);

		std::string modKey = mainIni.GetString("Main", key, "");

		std::string path = mainIni.GetString("Mods", modKey, "");
		if (path.empty()) {
			error("Couldn't find mod with ID: %s", modKey.c_str());
			continue;
		}

		INIReader modConfig(path);
		if (modConfig.ParseError() != 0) {
			error("Failed to load: %s - %s", path.c_str(), modConfig.ParseErrorMessage().c_str());
			continue;
		}

		auto* mod = new Mod{};
		mod->iniPath = path;
		mod->name = modConfig.GetString("Desc", "Name", "");
		auto iniPath = std::filesystem::path{ path };
		auto folderPath = iniPath.parent_path();
		mod->dataFolder = folderPath;
		mods.push_back(mod);

		info("Loaded: %s", mod->name.c_str());

		std::string dllFiles = modConfig.GetString("Main", "DLLFile", "");
		if (!dllFiles.empty()) {
			SetDllDirectoryA(folderPath.string().c_str());
			SetCurrentDirectoryA(folderPath.string().c_str());

			std::stringstream stream{ dllFiles };
			std::string dllName;
			while (std::getline(stream, dllName, ',')) {
				info("Loading DLL: %s", dllName.c_str());
				HMODULE module = LoadLibraryA((folderPath / dllName).string().c_str());
				if (!module) {
					DWORD errorV = GetLastError();
                    LPSTR msgBuffer = nullptr;

                    DWORD msgSize = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL, errorV, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuffer, 0, NULL);

					std::string message{ msgBuffer, msgSize };
					error("Failed to load %s: %s", dllName.c_str(), message.c_str());
				}
				else {
					info("Loaded DLL: %s", (folderPath / dllName).string().c_str());
				}
			}
		}
	}

	SetDllDirectoryA(dllDir);
	SetCurrentDirectoryA(curDir);
	delete[] dllDir;
	delete[] curDir;
}

bool ModLoader::moddedFileExists(char* filepath) {
	std::string utf8 = convertToUtf8(filepath);
	for (auto* mod : mods) {
		std::filesystem::path modFilepath{ mod->dataFolder };
		modFilepath /= utf8;
		std::string ogModFilePath{ convertFromUtf8(modFilepath.string()) };
		if (fileExists(ogModFilePath.c_str()))
			return true;
	}
	return false;
}

std::string ModLoader::getModdedFilepath(char* filepath) {
	std::string utf8 = convertToUtf8(filepath);
	for (auto* mod : mods) {
		std::filesystem::path modFilepath{ mod->dataFolder };
		modFilepath /= utf8;
		std::string ogModFilePath{ convertFromUtf8(modFilepath.string()) };
		if (fileExists(ogModFilePath.c_str()))
			return ogModFilePath;
	}
	return nullptr;
}
