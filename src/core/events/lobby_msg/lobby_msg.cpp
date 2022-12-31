#include "dependencies/std_include.hpp"
#include "lobby_msg.hpp"

namespace events::lobby_msg
{
	bool log_messages = true;

	namespace
	{
		bool handle_host_disconnect_client(const game::netadr_t& from, game::msg_t& msg, game::LobbyModule module)
		{
			PRINT_MESSAGE("LobbyMSG", "Disconnect prevented from %s", utils::get_sender_string(from).data());
			return true;
		}
		
		using callback = std::function<bool(const game::netadr_t&, game::msg_t&, game::LobbyModule module)>;
		using pair = std::pair<game::LobbyModule, game::MsgType>;

		auto& get_callbacks()
		{
			static std::unordered_map<pair, callback> callbacks{};
			return callbacks;
		}

		void on_message(const game::LobbyModule module, const game::MsgType type, const callback& callback)
		{
			get_callbacks()[{ module, type }] = callback;
		}

		bool handle_packet(const game::netadr_t& from, game::msg_t* msg)
		{
			const auto module = game::LobbyModule(msg->read<uint8_t>());
			msg->read<uint8_t>();
			
			if (!game::call<bool>(offsets::LobbyMsgRW_PrepReadMsg_, msg))
				return true;

			const auto ip_str{ utils::get_sender_string(from) }; 
			const auto type_name{ game::LobbyTypes_GetMsgTypeName(msg->type) };

			if (log_messages)
				PRINT_LOG("Received lobby message [%i] <%s> from %s", module, type_name, ip_str.data());

			const auto& callbacks = get_callbacks();
			const auto handler = callbacks.find({ module, msg->type });

			if (handler == callbacks.end())
				return false;

			return handler->second(from, *msg, module);
		}

		uint16_t __fastcall big_short(uint16_t value, uintptr_t* rsp, game::msg_t* msg)
		{
			if (*(rsp + 16) == OFFSET(offsets::ret_msg_read_short))
			{
				const auto retn_address = *(rsp + 16 + 6); 
				
				const std::vector<std::pair<uintptr_t, game::netadr_t>> addresses =
				{
					{ OFFSET(offsets::ret_handle_packet_1), *reinterpret_cast<game::netadr_t*>(rsp + 16 + 6 + 19) },
					{ OFFSET(offsets::ret_handle_packet_2), *reinterpret_cast<game::netadr_t*>(rsp + 16 + 6 + 41) },
					{ OFFSET(offsets::ret_handle_packet_3), {} },
				}; 

				const auto result = std::find_if(addresses.begin(), addresses.end(), [&](const auto& address) { return address.first == retn_address; });

				if (result != addresses.end())
				{
					const auto msg_backup = *msg;

					if (const auto header = msg->read<uint16_t>(); header == 0x97A)
					{
						if (lobby_msg::handle_packet(result->second, msg))
						{
							msg->type = static_cast<game::MsgType>(game::MESSAGE_TYPE_NONE);
						}
						else
						{
							*msg = msg_backup;
						}
					}
				}
			}

			return value;
		}

		int __fastcall big_long_read(int value, uintptr_t* rsp, game::msg_t* msg)
		{
			if (*(rsp + 16) == OFFSET(offsets::ret_msg_read_long))
			{
				const auto retn_address = *(rsp + 16 + 6);
				
				if (retn_address == OFFSET(offsets::ret_lobby_msg_rw_package_int))
				{
					const auto* key = reinterpret_cast<const char*>(*(rsp + 16 + 6 - 1));

					const std::vector<std::pair<std::string, uint32_t>> patches =
					{
						{ "lobbytype", 3 },
					};

					const auto result = std::any_of(patches.begin(), patches.end(), [&](const auto& p) { return p.first == key && value >= p.second; });

					if (result)
					{
						PRINT_LOG("Crash attempt caught <%s> with key '%s' of value [%i]", game::LobbyTypes_GetMsgTypeName(msg->type), key, value);
						msg->overflowed = 1;
					}
				}
			}
			
			return value;
		}
	}
	
	void initialize()
	{
		const auto big_short_stub = utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.pushad64();
			a.lea(r8, qword_ptr(rbx));
			a.lea(rdx, qword_ptr(rsp));
			a.call_aligned(big_short);
			a.popad64(true);
			a.ret();
		});

		const auto big_long_read_stub = utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.pushad64();
			a.lea(r8, qword_ptr(rbx));
			a.lea(rdx, qword_ptr(rsp));
			a.call_aligned(big_long_read);
			a.popad64(true);
			a.ret();
		});

		scheduler::on_game_initialized([=]()
		{
			utils::hook::set(OFFSET(offsets::BigShort), big_short_stub);
			utils::hook::set(OFFSET(offsets::BigLong), big_long_read_stub);
		}, scheduler::pipeline::main);

		lobby_msg::on_message(game::LOBBY_MODULE_CLIENT, game::MESSAGE_TYPE_LOBBY_HOST_DISCONNECT_CLIENT, lobby_msg::handle_host_disconnect_client);
	}
}