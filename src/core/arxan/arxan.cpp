#include "dependencies/std_include.hpp"
#include "arxan.hpp"

namespace arxan
{
	utils::hook::detour get_thread_context_hook;

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
	
	void initialize()
	{
		get_thread_context_hook.create(utils::nt::library("kernelbase.dll").get_proc<void*>("GetThreadContext"), get_thread_context);
	}
}