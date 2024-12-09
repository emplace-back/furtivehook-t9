#include "dependencies/stdafx.hpp"
#include "events.hpp"

namespace events
{
	utils::hook::detour leave_critical_section_hook; 
	utils::hook::detour time_get_time_hook;
	bool prevent_join = true;

	uint32_t little_long(uint32_t value, uintptr_t* rsp)
	{
		const auto retaddr = *(rsp + 16 + 6); 

		if (!utils::nt::library::get_by_address(retaddr))
			return value;

		const auto msg = *reinterpret_cast<game::msg_t**>(rsp + 8 + 4);

		// LobbyMsgRW_PackageInt/LobbyMsgRW_PackageUInt
		if (*reinterpret_cast<uint64_t*>(retaddr) == 0x8B4800357B800689)
		{
			const auto* key = reinterpret_cast<const char*>(*(rsp + 16 + 5));

			if (lobby_msg::handle<uint32_t>(value, key))
			{
				PRINT_LOG("Crash attempt caught <%s> with key '%s' of value [%i]", game::LobbyTypes_GetMsgTypeName(msg->type), key, value);
				msg->overflowed = 1;
			}
		}
		else
		{
			const std::vector<std::pair<uintptr_t, bool>> oob_handlers =
			{
				{ 0x6A8AE8CE8B0C558B, false }, // Netchan_DispatchOOBPackets -> CL_ConnectionlessPacket
				{ 0x418FF18E80C4F8B, true },  //  Netchan_DispatchOOBPackets -> SV_ConnectionlessPacket
			};

			const auto oob = std::find_if(oob_handlers.begin(), oob_handlers.end(), [=](const auto& handler) { return *reinterpret_cast<uint64_t*>(retaddr) == handler.first; });

			if (oob != oob_handlers.end())
			{
				const auto netadr = *reinterpret_cast<game::netadr_t*>(*(rsp + 16 + 8));

				msg->readcount += sizeof(int);

				if (events::connectionless_packet::handle_command(netadr, msg, oob->second))
				{
					msg->cursize = 0;
				}
				else
				{
					msg->readcount -= sizeof(int);
				}
			}
		}

		return value;
	}

	DWORD time_get_time(uintptr_t* rsp)
	{
		const auto result = time_get_time_hook.call<DWORD>();
		const auto retaddr = *(rsp + 16 + 6); 
		
		if (!utils::nt::library::get_by_address(retaddr))
			return result;
		
		// Com_Frame_Try_Block_Function
		if ((*reinterpret_cast<uint64_t*>(retaddr) & 0xFFFFFFFFFF) == 0x828D0FC73B)
		{
			scheduler::execute(scheduler::pipeline::main);
		}

		return result;
	}

	void leave_critical_section(LPCRITICAL_SECTION section, uintptr_t* rsp, uintptr_t r15)
	{
		const auto retaddr = *(rsp + 16 + 6);

		if (*reinterpret_cast<uint64_t*>(retaddr) == 0x245C8B48C5B60F40)
		{
			const auto msg = *reinterpret_cast<game::msg_t**>(*(rsp + 8 + 1));
			const auto message = *reinterpret_cast<game::NetChanMessage_s**>(*(rsp + 16 + 7));
			const auto type = static_cast<game::NetChanMsgType>(r15 / sizeof(uint64_t));

			const auto msg_backup = *msg;

			if (game::net::netchan::get(message, msg, type))
			{
				msg->cursize = 0;
			}
			else
			{
				*msg = msg_backup;
			}
		}
		else if (retaddr == OFFSET(offsets::ret_com_switch_mode))
		{
			game::cmd_text = reinterpret_cast<game::CmdText*>(*(rsp + 16 + 7));
		}

		return leave_critical_section_hook.call(section);
	}
	
	void initialize()
	{
		lobby_msg::initialize();
		instant_message::initialize();
		connectionless_packet::initialize(); 
		
		time_get_time_hook.create(::timeGetTime, utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.pushad64();
			a.lea(rcx, qword_ptr(rsp));
			a.call_aligned(time_get_time);
			a.popad64(true);
			a.ret();
		}));

		utils::hook::iat("kernel32.dll", "LeaveCriticalSection", utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.mov(r8, r15);
			a.pushad64();
			a.lea(rdx, qword_ptr(rsp));
			a.call_aligned(leave_critical_section);
			a.popad64(true);
			a.ret();
		}));
		
		scheduler::once([]()
		{
			const auto littlelong_ptr = utils::hook::scan_pattern(signatures::littlelong_ptr);

			if (!littlelong_ptr)
				return;

			const auto little_long_stub = utils::hook::assemble([](utils::hook::assembler& a)
			{
				a.pushad64();
				a.lea(rdx, qword_ptr(rsp));
				a.call_aligned(little_long);
				a.popad64(true);
				a.ret();
			}); 
			
			scheduler::loop([=]()
			{
				const auto littlelong = utils::hook::extract<uintptr_t**>(utils::hook::extract<uint8_t*>(littlelong_ptr + 1) + 3);

				if (!littlelong)
					return;

				if (*littlelong == little_long_stub)
					return;
				
				utils::hook::set(littlelong, little_long_stub);
			});
		}, scheduler::pipeline::main);
	}
}