#include "dependencies/std_include.hpp"
#include "exception.hpp"

namespace exception
{
	utils::hook::detour rtl_dispatch_exception_hook; 
	uint8_t* call_rtl_dispatch_exception_ptr{};
	std::mutex exception_mutex;
	
	namespace
	{
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

				utils::nt::terminate(code);
			}

			return true;
		}

		bool __stdcall rtl_dispatch_exception(PEXCEPTION_RECORD ex, PCONTEXT ctx)
		{
			if (_ReturnAddress() == call_rtl_dispatch_exception_ptr + 5)
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
		call_rtl_dispatch_exception_ptr = utils::hook::scan_pattern("ntdll.dll", signatures::call_rtl_dispatch_exception_ptr);

		if (!call_rtl_dispatch_exception_ptr)
			return;

		rtl_dispatch_exception_hook.create(utils::hook::extract(call_rtl_dispatch_exception_ptr + 1), rtl_dispatch_exception);

		dvars::initialize();
	}
}