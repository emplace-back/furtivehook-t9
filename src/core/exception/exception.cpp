#include "dependencies/std_include.hpp"
#include "exception.hpp"

namespace exception
{
	utils::hook::detour rtl_dispatch_exception_hook; 
	uint8_t* ret_rtl_dispatch_exception_ptr{};
	std::mutex exception_mutex;
	
	namespace
	{
		bool is_exception_in_game_module(const uintptr_t address, const uintptr_t base)
		{
			utils::nt::library self{};
			return address >= base && address < base - 0x1000 + self.get_nt_headers()->OptionalHeader.SizeOfImage;
		}
		
		bool handle_exception(const EXCEPTION_POINTERS* ex)
		{
			const auto code = ex->ExceptionRecord->ExceptionCode;
			const auto addr = ex->ContextRecord->Rip;
			const auto base = game::get_base();

			if (!ex->ExceptionRecord->ExceptionInformation[0])
				return false;

			if (code < STATUS_ACCESS_VIOLATION || code == 0xE06D7363)
				return false;

			const std::lock_guard<std::mutex> _(exception_mutex);

			std::string message;

			if (is_exception_in_game_module(addr, base))
			{
				message = utils::string::va("Exception: 0x%08X at 0x%llX", code, game::derelocate(addr));
			}
			else
			{
				message = utils::string::va("Exception: 0x%08X at 0x%llX (outside of game module)", code, addr);
			}

			PRINT_LOG("%s", message.data());

			if (code == STATUS_ACCESS_VIOLATION)
			{
				MessageBoxA(nullptr, message.data(), "Exception", MB_ICONERROR);

				utils::nt::terminate(code);
			}

			return true;
		}

		bool __stdcall rtl_dispatch_exception(PEXCEPTION_RECORD ex, PCONTEXT ctx)
		{
			if (_ReturnAddress() == ret_rtl_dispatch_exception_ptr)
			{
				EXCEPTION_POINTERS pex
				{
					ex,
					ctx
				};

				if (exception::handle_exception(&pex))
				{
					return true;
				}
			}

			return rtl_dispatch_exception_hook.call<bool>(ex, ctx);
		}
	}

	void initialize()
	{
		auto rtl_dispatch_exception_ptr = utils::hook::scan_pattern("ntdll.dll", "40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC D0");
		ret_rtl_dispatch_exception_ptr = utils::hook::scan_pattern("ntdll.dll", "84 C0 74 0C 48 8B CC");

		if (!rtl_dispatch_exception_ptr)
			return;

		rtl_dispatch_exception_hook.create(rtl_dispatch_exception_ptr, rtl_dispatch_exception);

		scheduler::once([]()
		{
			//*(int*)0 = 0;
		}, scheduler::pipeline::main);
	}
}