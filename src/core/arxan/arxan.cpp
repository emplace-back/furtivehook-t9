#include "dependencies/std_include.hpp"
#include "arxan.hpp"

#define ProcessImageFileNameWin32 43

namespace arxan
{
	utils::hook::detour get_thread_context_hook;
	utils::hook::detour create_mutex_hook;
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
			L"Wireshark"s,
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

	BOOL get_thread_context(HANDLE thread_handle, LPCONTEXT context)
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

	HANDLE __stdcall create_mutex(LPSECURITY_ATTRIBUTES attributes, const char* name, DWORD flags, DWORD access)
	{
		if (name == "$ IDA trusted_idbs"s || name == "$ IDA registry mutex $"s)
		{
			flags = 0;
		}

		return create_mutex_hook.call<HANDLE>(attributes, name, flags, access);
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
		nt_query_information_process_hook.create(ntdll.get_proc<void*>("NtQueryInformationProcess"), nt_query_information_process);
		nt_query_system_information_hook.create(ntdll.get_proc<void*>("NtQuerySystemInformation"), nt_query_system_information);
		nt_query_system_information_hook.move();

		get_thread_context_hook.create(utils::nt::library("kernelbase.dll").get_proc<void*>("GetThreadContext"), get_thread_context);

		create_mutex_hook.create(CreateMutexExA, create_mutex);
	}
}