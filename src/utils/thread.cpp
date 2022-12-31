#include "dependencies/std_include.hpp"
#include "thread.hpp"

namespace utils::thread
{
	std::vector<DWORD> get_thread_ids()
	{
		nt::handle<INVALID_HANDLE_VALUE> h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
		if (!h)
		{
			return {};
		}

		THREADENTRY32 entry{};
		entry.dwSize = sizeof(entry);

		if (!Thread32First(h, &entry))
		{
			return {};
		}

		std::vector<DWORD> ids{};

		do
		{
			if (entry.th32OwnerProcessID != GetCurrentProcessId() || entry.th32ThreadID == GetCurrentThreadId())
				continue;

			ids.emplace_back(entry.th32ThreadID);
		} while (Thread32Next(h, &entry));

		return ids;
	}
}