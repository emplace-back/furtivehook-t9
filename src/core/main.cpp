#include "dependencies/stdafx.hpp"
#include <iphlpapi.h>

BOOL __stdcall DllMain(const HMODULE module, const DWORD reason, const LPVOID /*reserved*/)
{
	switch (reason) 
	{
		case DLL_PROCESS_ATTACH:
		{
			try 
			{
				DisableThreadLibraryCalls(module);

				input::initialize();
				events::initialize();
				misc::initialize();
				friends::initialize();
			}
			catch (const std::exception& ex) 
			{
				MessageBoxA(nullptr, ex.what(), "Exception Caught", 0);
			}
			catch (...) 
			{
				MessageBoxA(nullptr, "Something unexpected happened", "Exception Caught!", 0);
			}

			break;
		}
		
		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}
