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

		int __fastcall big_long_read(int value, uintptr_t* rsp, game::msg_t* msg)
		{
			const auto ret_address = *(rsp + 16 + 6);

			if (ret_address == OFFSET(offsets::ret_lobby_msg_rw_package_int))
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
			
			return value;
		}
	}

	bool __fastcall handle_packet(const game::LobbyModule module, const game::netadr_t& from, game::msg_t& msg)
	{
		const auto ip_str{ utils::get_sender_string(from) };
		const auto type_name{ game::LobbyTypes_GetMsgTypeName(msg.type) };

		if (log_messages)
			PRINT_LOG("Received lobby message [%i] <%s> from %s", module, type_name, ip_str.data());

		const auto& callbacks = get_callbacks();
		const auto handler = callbacks.find({ module, msg.type });

		if (handler == callbacks.end())
			return false;

		return handler->second(from, msg, module);
	}
	
	void initialize()
	{
		const auto big_long_read_stub = utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.pushad64();
			a.lea(r8, qword_ptr(rbx));
			a.lea(rdx, qword_ptr(rsp));
			a.call_aligned(big_long_read);
			a.popad64(true);
			a.ret();
		});

		scheduler::loop([=]()
		{
			if (const auto biglong = reinterpret_cast<uintptr_t**>(OFFSET(offsets::BigLong)); 
				biglong && *biglong != big_long_read_stub)
			{
				DEBUG_LOG("Hooking BigLong");
				utils::hook::set(biglong, big_long_read_stub);
			}
		}, scheduler::pipeline::main);

		lobby_msg::on_message(game::LOBBY_MODULE_CLIENT, game::MESSAGE_TYPE_LOBBY_HOST_DISCONNECT_CLIENT, lobby_msg::handle_host_disconnect_client);
	}
}