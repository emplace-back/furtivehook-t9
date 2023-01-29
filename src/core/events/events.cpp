#include "dependencies/stdafx.hpp"
#include "events.hpp"

namespace events
{
	utils::hook::detour time_get_time_hook;
	bool prevent_join = true;

	int __fastcall big_long(int value, uintptr_t* rsp, game::msg_t* msg)
	{
		static uint8_t* lobby_msg_rw_package_int{}, *msg_read_long;

		if (!lobby_msg_rw_package_int)
			lobby_msg_rw_package_int = utils::hook::scan_pattern(signatures::ret_lobby_msg_rw_package_int);

		if (!msg_read_long)
			msg_read_long = utils::hook::scan_pattern(signatures::msg_read_long);

		if (reinterpret_cast<uint8_t*>(*(rsp + 16)) == msg_read_long + 0x4F)
		{
			const auto retaddr = *(rsp + 22);

			if (reinterpret_cast<uint8_t*>(retaddr) == lobby_msg_rw_package_int + 0x7C)
			{
				const auto* key = reinterpret_cast<const char*>(*(rsp + 22 - 1));
				
				if (lobby_msg::handle<uint32_t>(value, key))
				{
					PRINT_LOG("Crash attempt caught <%s> with key '%s' of value [%i]", game::LobbyTypes_GetMsgTypeName(msg->type), key, value);
					msg->overflowed = 1;
				}
			}

			const std::vector<std::pair<uintptr_t, uintptr_t>> oob_handlers =
			{
				{ 0xCE8B0C558B, *(rsp + 16 + 6 + 2) },
				{ 0xCE8B0C578B, *(rsp + 16 - 6) },
			};

			const auto oob = std::find_if(oob_handlers.begin(), oob_handlers.end(), [=](const auto& handler) 
			{ 
				return (*reinterpret_cast<uint64_t*>(retaddr) & 0xFFFFFFFFFF) == handler.first; 
			});

			if (oob != oob_handlers.end())
			{
				if (events::connectionless_packet::handle_command(
					*reinterpret_cast<game::netadr_t*>(oob->second), 
					msg))
				{
					*msg = {};
				}
			}
		}

		return value;
	}

	DWORD __stdcall time_get_time(uintptr_t* rsp)
	{
		static uint8_t* ret_com_client_packet_event{};

		if (!ret_com_client_packet_event)
		{
			ret_com_client_packet_event = utils::hook::scan_pattern(signatures::ret_com_client_packet_event);
		}

		// Sys_Milliseconds
		if ((*reinterpret_cast<uint64_t*>(*(rsp + 16) + 6) & 0xFFFFFFFFFF) == 0xC328C48348)
		{
			const auto retaddr = *(rsp + 22);

			// Com_Frame_Try_Block_Function
			if ((*reinterpret_cast<uint64_t*>(retaddr) & 0xFFFFFFFFFF) == 0x828D0FC73B)
			{
				scheduler::execute(scheduler::pipeline::main);
			}
			else if (reinterpret_cast<uint8_t*>(retaddr) == ret_com_client_packet_event)
			{
				const auto msg = reinterpret_cast<game::msg_t*>(rsp + 16 + 6 + 11);
				const auto msg_backup = *msg;

				if (!events::connectionless_packet::handle_command(
					*reinterpret_cast<game::netadr_t*>(rsp + 16 + 6 + 9),
					msg,
					false))
				{
					*msg = msg_backup;
				}
			}
		}

		return time_get_time_hook.call<DWORD>();
	}

	void leave_critical_section(LPCRITICAL_SECTION section, uintptr_t* rsp)
	{
		if (*(rsp + 16 + 6) == OFFSET(offsets::ret_com_switch_mode))
		{
			game::cmd_text = reinterpret_cast<game::CmdText*>(*(rsp + 16 + 6 + 1));
		}

		return LeaveCriticalSection(section);
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
			a.pushad64();
			a.lea(rdx, qword_ptr(rsp));
			a.call_aligned(leave_critical_section);
			a.popad64(true);
			a.ret();
		}));
		
		scheduler::once([]()
		{
			const auto biglong_ptr = utils::hook::scan_pattern(signatures::biglong_ptr);

			if (!biglong_ptr)
				return;

			const auto big_long_stub = utils::hook::assemble([](utils::hook::assembler& a)
			{
				a.pushad64();
				a.lea(r8, qword_ptr(rbx));
				a.lea(rdx, qword_ptr(rsp));
				a.call_aligned(big_long);
				a.popad64(true);
				a.ret();
			}); 
			
			scheduler::loop([=]()
			{
				const auto biglong = utils::hook::extract<uintptr_t**>(utils::hook::extract<uint8_t*>(biglong_ptr + 1) + 3);

				if (!biglong)
					return;

				if (*biglong == big_long_stub)
					return;
				
				utils::hook::set(biglong, big_long_stub);
			});
		}, scheduler::pipeline::main);
	}
}