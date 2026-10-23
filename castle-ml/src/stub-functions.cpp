#include <cassert>
#include <windows.h>
#include <detours.h>

// Taken from https://github.com/hedge-dev/HE1ML/blob/main/Source/StubFunctions.cpp

#define MAKE_STUB(NAME) void __declspec(dllexport) NAME() { MessageBoxA(nullptr, "Attempted to call unloaded function: " #NAME, "CML", MB_ICONERROR); }

extern "C"
{
	// d3d8.dll
	MAKE_STUB(Direct3DCreate8);
}

void ResolveStubMethods(void* module)
{
#define MAP(MODULE, ADDRESS) ((char*)(MODULE) + (unsigned)(ADDRESS))

	HMODULE executingModule{};
	const BOOL result = GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCSTR>(ResolveStubMethods), &executingModule);
	assert(result);

	const auto* header = reinterpret_cast<IMAGE_DOS_HEADER*>(executingModule);
	const auto* ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(MAP(executingModule, header->e_lfanew));
	
	const auto* exportDir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(MAP(executingModule, ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress));

	const DWORD* names = reinterpret_cast<DWORD*>(MAP(executingModule, exportDir->AddressOfNames));
	for (size_t i = 0; i < exportDir->NumberOfNames; i++)
	{
		const char* name = MAP(executingModule, names[i]);
		void* newProc = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(module), name));

		if (newProc)
		{
			const DWORD* functions = reinterpret_cast<DWORD*>(MAP(executingModule, exportDir->AddressOfFunctions));
			const WORD* ordinals = reinterpret_cast<WORD*>(MAP(executingModule, exportDir->AddressOfNameOrdinals));

			const DWORD function = functions[ordinals[i]];
			void* oldProc = MAP(executingModule, function);
			
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&oldProc, newProc);
			DetourTransactionCommit();
		}
	}
	
#undef MAP
}
