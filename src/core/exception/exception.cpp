#include "dependencies/std_include.hpp"
#include "exception.hpp"

namespace exception
{
	utils::hook::detour rtl_dispatch_exception_hook; 
	std::mutex exception_mutex;
	
	namespace
	{
		constexpr MINIDUMP_TYPE get_minidump_type()
		{
			const auto type = MiniDumpIgnoreInaccessibleMemory
				| MiniDumpWithHandleData
				| MiniDumpScanMemory
				| MiniDumpWithProcessThreadData
				| MiniDumpWithFullMemoryInfo
				| MiniDumpWithThreadInfo
				| MiniDumpWithUnloadedModules;

			return static_cast<MINIDUMP_TYPE>(type);
		}

		bool write(const LPEXCEPTION_POINTERS ex)
		{
			const auto file_path = utils::string::get_log_file("minidumps", "dmp");
			if (!utils::io::write_file(file_path, "Creating minidump .."))
			{
				return false;
			}

			const auto file_handle = CreateFileA(file_path.data(), GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				nullptr);

			MINIDUMP_EXCEPTION_INFORMATION minidump_exception_info = { GetCurrentThreadId(), ex, FALSE };

			return MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file_handle, get_minidump_type(), &minidump_exception_info, nullptr, nullptr);
		}
		
		bool handle_exception(PEXCEPTION_POINTERS ex)
		{
			const auto code = ex->ExceptionRecord->ExceptionCode;
			const auto addr = ex->ContextRecord->Rip;
			const auto base = game::get_base();

			if (utils::nt::is_shutdown_in_progress())
				return false;
			
			if (code == STATUS_ILLEGAL_INSTRUCTION || code == STATUS_PRIVILEGED_INSTRUCTION || code < STATUS_ACCESS_VIOLATION || code == 0xe06d7363)
				return false;

			if (ex->ExceptionRecord->NumberParameters > 1 && ex->ExceptionRecord->ExceptionInformation[1] == 0xFFFFFFFFFFFFFFFF)
				return false;

			if (dvars::handle_exception(ex))
				return true;

			const std::lock_guard<std::mutex> _(exception_mutex);

			const auto game = utils::nt::library{};
			const auto source_module = utils::nt::library::get_by_address(reinterpret_cast<void*>(addr));

			std::string message;
			
			if (source_module == game)
			{
				message = utils::string::va("Exception: 0x%08X at 0x%llX", code, game::derelocate(addr));
			}
			else
			{
				message = utils::string::va("Exception: 0x%08X at 0x%llX (outside of game module) %s", code, addr, source_module.get_name().data());
			}

			PRINT_LOG("%s", message.data());

			if (code == STATUS_ACCESS_VIOLATION)
			{
				MessageBoxA(nullptr, message.data(), "Exception", MB_ICONERROR);

				if (!write(ex))
				{
					MessageBoxA(nullptr, utils::string::va("There was an error creating the minidump! (0x%08X)", GetLastError()).data(), "Minidump Error", MB_OK | MB_ICONERROR);
				}

				utils::nt::terminate(code);
			}

			return true;
		}

		bool __stdcall rtl_dispatch_exception(PEXCEPTION_RECORD ex, PCONTEXT ctx)
		{
			// KiUserExceptionDispatcher
			if ((*reinterpret_cast<uint64_t*>(_ReturnAddress()) & 0xFFFFFFFFFFFFFF) == 0xCC8B480C74C084)
			{
				auto& pex = EXCEPTION_POINTERS
				{ 
					ex, 
					ctx 
				};

				if (exception::handle_exception(&pex))
					return true;
			}

			return rtl_dispatch_exception_hook.call<bool>(ex, ctx);
		}
	}

	void initialize()
	{
		const auto call_rtl_dispatch_exception_ptr = utils::nt::library("ntdll.dll").get_proc<uint8_t*>("KiUserExceptionDispatcher");

		if (!call_rtl_dispatch_exception_ptr)
			return;

		rtl_dispatch_exception_hook.create(utils::hook::extract(call_rtl_dispatch_exception_ptr + 0x29 + 1), rtl_dispatch_exception);

		dvars::initialize();
	}
}