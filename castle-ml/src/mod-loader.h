#pragma once
#include <windows.h>
#include <filesystem>
#include <vector>

struct Mod {
    std::string name;
    std::filesystem::path iniPath;
    std::filesystem::path dataFolder;
};

class ModLoader {
public:
    std::filesystem::path modFolder{};
    std::vector<Mod*> mods{};

    void init(HMODULE module);
    bool moddedFileExists(char* filepath);
    std::string getModdedFilepath(char* filepath);
};
