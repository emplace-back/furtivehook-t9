#include "dependencies/stdafx.hpp"
#include "arxan.hpp"

#define ProcessImageFileNameWin32 43

namespace arxan
{
	utils::hook::detour get_thread_context_hook;
	utils::hook::detour nt_create_mutant_hook;
	utils::hook::detour get_window_text_a_hook;
	utils::hook::detour nt_query_system_information_hook;
	utils::hook::detour nt_query_information_process_hook;
	
	bool remove_keyword_from_string(const UNICODE_STRING& string)
	{
		if (!string.Buffer || !string.Length)
		{
			return false;
		}

		const auto keywords =
		{
			L"IDA"s,
			L"ida"s,
		};

		const auto path = std::wstring_view{ string.Buffer, string.Length / sizeof(*string.Buffer) };

		auto modified{ false };

		for (const auto& keyword : keywords)
		{
			while (true)
			{
				const auto pos = path.find(keyword);
				if (pos == std::wstring::npos)
				{
					break;
				}

				modified = true;

				for (size_t i = 0; i < keyword.size(); ++i)
				{
					string.Buffer[pos + i] = L'a';
				}
			}
		}

		return modified;
	}

	bool remove_keyword_from_string(wchar_t* str, const size_t length)
	{
		UNICODE_STRING unicode_string{};
		unicode_string.Buffer = str;
		unicode_string.Length = static_cast<uint16_t>(length);
		unicode_string.MaximumLength = unicode_string.Length;

		return remove_keyword_from_string(unicode_string);
	}
	
	BOOL __stdcall get_thread_context(HANDLE thread_handle, LPCONTEXT context)
	{
		constexpr auto debug_registers_flag = (CONTEXT_DEBUG_REGISTERS & ~CONTEXT_AMD64);

		if (context && context->ContextFlags & debug_registers_flag)
		{
			const auto* source = _ReturnAddress();
			const auto game = utils::nt::library{};
			const auto source_module = utils::nt::library::get_by_address(source);

			if (source_module == game)
			{
				context->ContextFlags &= ~debug_registers_flag;
			}
		}

		return get_thread_context_hook.call<BOOL>(thread_handle, context);
	}

	NTSTATUS __stdcall nt_create_mutant(PHANDLE handle, ACCESS_MASK access, POBJECT_ATTRIBUTES attributes, BOOLEAN owner)
	{
		if (attributes)
		{
			remove_keyword_from_string(*attributes->ObjectName);
		}
		
		return nt_create_mutant_hook.call<NTSTATUS>(handle, access, attributes, owner);
	}

	int __stdcall get_window_text_a(HWND window, char* string, size_t max_count)
	{
		std::wstring wstr{};
		wstr.resize(max_count);

		const auto count = GetWindowTextW(window, wstr.data(), max_count);

		if (count)
		{
			remove_keyword_from_string(wstr.data(), count);

			const std::string regular_str(wstr.begin(), wstr.end());
			memset(string, 0, max_count);
			memcpy(string, regular_str.data(), count);
		}

		return count;
	}

	NTSTATUS __stdcall nt_query_information_process(HANDLE handle, PROCESSINFOCLASS info_class, void* info, uint32_t info_length, uint32_t* return_length)
	{
		const auto status = nt_query_information_process_hook.call<NTSTATUS>(handle, info_class, info, info_length, return_length);

		if (NT_SUCCESS(status))
		{
			if (info_class == ProcessImageFileName || static_cast<int>(info_class) == ProcessImageFileNameWin32)
			{
				remove_keyword_from_string(*static_cast<UNICODE_STRING*>(info));
			}
		}

		return status;
	}

	NTSTATUS __stdcall nt_query_system_information(SYSTEM_INFORMATION_CLASS system_information_class, void* system_information, uint32_t system_information_length, uint32_t* return_length)
	{
		const auto status = nt_query_system_information_hook.call<NTSTATUS>(system_information_class, system_information, system_information_length, return_length);

		if (NT_SUCCESS(status))
		{
			if (system_information_class == SystemProcessInformation && !utils::nt::is_shutdown_in_progress())
			{
				auto addr = static_cast<uint8_t*>(system_information);

				while (true)
				{
					const auto info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(addr);
					remove_keyword_from_string(info->ImageName);

					if (!info->NextEntryOffset)
					{
						break;
					}

					addr += info->NextEntryOffset;
				}
			}
		}

		return status;
	}
	
	void initialize()
	{
		const utils::nt::library ntdll("ntdll.dll");
		nt_create_mutant_hook.create(ntdll.get_proc<uint8_t*>("NtCreateMutant"), nt_create_mutant); 
		nt_query_information_process_hook.create(ntdll.get_proc<uint8_t*>("NtQueryInformationProcess"), nt_query_information_process);
		nt_query_system_information_hook.create(ntdll.get_proc<uint8_t*>("NtQuerySystemInformation"), nt_query_system_information);

		get_thread_context_hook.create(utils::nt::library("kernelbase.dll").get_proc<uint8_t*>("GetThreadContext"), get_thread_context);
		get_window_text_a_hook.create(::GetWindowTextA, get_window_text_a);
	}
}