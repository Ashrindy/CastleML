#pragma once
#include <windows.h>
#include <detours.h>
#include <cstdio>
#include <string>
#include <iconv.h>

#define PROC_ADDRESS(libraryName, procName) \
    GetProcAddress(LoadLibrary(TEXT(libraryName)), procName)

#define HOOK(returnType, callingConvention, functionName, location, ...) \
    typedef returnType callingConvention functionName(__VA_ARGS__); \
    functionName* original##functionName = (functionName*)(location); \
    returnType callingConvention implOf##functionName(__VA_ARGS__)

#define INSTALL_HOOK(functionName) \
{ \
    DetourTransactionBegin(); \
    DetourUpdateThread(GetCurrentThread()); \
    DetourAttach((void**)&original##functionName, (void*)implOf##functionName); \
    DetourTransactionCommit(); \
}

#define FUNCTION_PTR(returnType, callingConvention, function, location, ...) \
    returnType (callingConvention *function)(__VA_ARGS__) = (returnType(callingConvention*)(__VA_ARGS__))(location)

static bool fileExists(const char* path) {
    DWORD attributes = GetFileAttributes(path);

    return attributes != INVALID_FILE_ATTRIBUTES &&
           !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static HMODULE LoadSystemLibrary(const char* name) {
	std::string windir{};
	DWORD windirSize = GetSystemDirectoryA(nullptr, 0);

	windir.reserve(windirSize + strlen(name) + 1);
	windir.resize(windirSize + strlen(name));

	GetSystemDirectoryA(windir.data(), windirSize);

	windir[windirSize - 1] = '\\';
	strcpy(windir.data() + windirSize, name);

	return LoadLibraryA(windir.c_str());
}

static std::string convertToUtf8(const std::string& input) {
	iconv_t cd = iconv_open("UTF-8", "ISO-8859-15");

    std::string output(input.size() * 4 + 16, '\0');
    char* in = (char*)input.data();
    size_t inLeft = input.size();
    char* out = output.data();
    size_t outLeft = output.size();

    size_t result = iconv(cd, &in, &inLeft, &out, &outLeft);
    iconv_close(cd);

	output.resize(output.size() - outLeft);
    return output;
    return input;
}

static std::string convertFromUtf8(const std::string& input) {
	iconv_t cd = iconv_open("ISO-8859-15", "UTF-8");

    std::string output(input.size() * 2 + 16, '\0');
    char* in = (char*)input.data();
    size_t inLeft = input.size();
    char* out = output.data();
    size_t outLeft = output.size();

    size_t result = iconv(cd, &in, &inLeft, &out, &outLeft);
    iconv_close(cd);

    output.resize(output.size() - outLeft);
    return output;
    return input;
}
