#include "dependencies/std_include.hpp"
#include "misc.hpp"

namespace misc
{
	utils::hook::detour leave_critical_section_hook;
	void __stdcall leave_critical_section(LPCRITICAL_SECTION section, uintptr_t* rsp, int* event_type)
	{
		const auto source = *(rsp + 16);
		const auto game = utils::nt::library{};
		const auto source_module = utils::nt::library::get_by_address((void*)source); 
		
		if (source_module == game)
		{
			if (source == OFFSET(0x7FF700D81A7D))
			{
				const auto retn_address = *(rsp + 16 + 6);

				if (retn_address == OFFSET(0x7FF700D14B14))
				{
					if(*event_type > 0)
						DEBUG_LOG("%i", *event_type);
				}
			}
		}

		return leave_critical_section_hook.call(section);
	}
		
	void initialize()
	{
		const auto leave_critical_section_stub = utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.pushad64();
			a.mov(r8, rsi);
			a.lea(rdx, qword_ptr(rsp));
			a.call_aligned(leave_critical_section);
			a.popad64(true);
			a.ret();
		}); 

		leave_critical_section_hook.create(::LeaveCriticalSection, leave_critical_section_stub);
		
		scheduler::once(game::initialize, scheduler::pipeline::main);
	}
}