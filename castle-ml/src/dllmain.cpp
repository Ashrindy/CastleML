#include "mod-loader.h"
#include "helpers.h"
#include "logger.h"
#include <cstdio>

ModLoader modLoader{};

struct CFile {
	void** vftable;
	HANDLE fileHandle;
	int unk0;
	const char* filepath;
};

struct CString {
	char* value;
};

FUNCTION_PTR(CString*, __thiscall, CString_Ctor, 0x401470, CString*, const char*);

#define CSTRING_REPLACE(CSTRING) char* filepath = (CSTRING)->value; \
	if (modLoader.moddedFileExists(filepath)) { \
		auto moddedFilepath = modLoader.getModdedFilepath(filepath); \
		info("Loading modded file: %s", convertToUtf8(moddedFilepath).c_str()); \
		CString_Ctor(CSTRING, moddedFilepath.c_str()); \
	}


HOOK(char, __cdecl, sub_5BBE30, 0x5BBE30, CString* a1, int a2, int a3, int a4) {
	CSTRING_REPLACE(a1);
	return originalsub_5BBE30(a1, a2, a3, a4);
}

HOOK(int, __thiscall, sub_5BB960, 0x5BB960, int a1, CString a2, int a3) {
	CSTRING_REPLACE(&a2);
	return originalsub_5BB960(a1, a2, a3);
}

HOOK(int, __thiscall, sub_5BAE10, 0x5BAE10, int a1, CString a2, int a3, char a4, CString* a5, int a6) {
	CSTRING_REPLACE(&a2);
	return originalsub_5BAE10(a1, a2, a3, a4, a5, a6);
}

HOOK(int, __thiscall, sub_5BB2E0, 0x5BB2E0, int a1, CString a2, int a3, char a4, CString* a5) {
	CSTRING_REPLACE(&a2);
	return originalsub_5BB2E0(a1, a2, a3, a4, a5);
}

BOOL WINAPI DllMain(_In_ HINSTANCE instance, _In_ DWORD reason, _In_ LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		modLoader.init(instance);
		INSTALL_HOOK(sub_5BBE30);
		INSTALL_HOOK(sub_5BB960);
		INSTALL_HOOK(sub_5BAE10);
		INSTALL_HOOK(sub_5BB2E0);
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	default:
		break;
	}

	return TRUE;
}
