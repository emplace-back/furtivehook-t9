#include "dependencies/stdafx.hpp"

BOOL __stdcall DllMain(const HMODULE module, const DWORD reason, const LPVOID /*reserved*/)
{
	switch (reason) 
	{
		case DLL_PROCESS_ATTACH:
		{
			try 
			{
				DisableThreadLibraryCalls(module);

				arxan::initialize();
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

extern "C" __declspec(dllexport)
NTSTATUS __stdcall CallNtPowerInformation(POWER_INFORMATION_LEVEL info_level, void* input_buffer, ULONG input_buffer_length, void* output_buffer, ULONG output_buffer_length)
{
	static auto call_nt_power_information = [=]
	{
		char path[MAX_PATH] = { 0 };
		GetSystemDirectoryA(path, sizeof path);

		const auto powrprof = utils::nt::library::load(path + "/powrprof.dll"s);
		return powrprof.get_proc<decltype(&CallNtPowerInformation)>("CallNtPowerInformation");
	}(); 
	
	return call_nt_power_information(info_level, input_buffer, input_buffer_length, output_buffer, output_buffer_length);
}
