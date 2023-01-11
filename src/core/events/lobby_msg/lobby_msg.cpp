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
			static uint8_t* ret_lobby_msg_rw_package_int{};

			if (!ret_lobby_msg_rw_package_int)
			{
				ret_lobby_msg_rw_package_int = utils::hook::scan_pattern(signatures::ret_lobby_msg_rw_package_int) + 0x7C;
			}

			const auto ret_address = *(rsp + 16 + 6);

			if (reinterpret_cast<uint8_t*>(ret_address) == ret_lobby_msg_rw_package_int)
			{
				const auto* key = reinterpret_cast<const char*>(*(rsp + 16 + 6 - 1));

				const std::vector<std::pair<std::string, uint32_t>> patches =
				{
					{ "lobbytype", 2 },
					{ "clientcount", 18 },
				};

				const auto result = std::any_of(patches.begin(), patches.end(), [&](const auto& p) { return p.first == key && static_cast<uint32_t>(value) > p.second; });

				if (result)
				{
					PRINT_LOG("Crash attempt caught <%s> with key '%s' of value [%i]", game::LobbyTypes_GetMsgTypeName(msg->type), key, value);
					msg->overflowed = 1;
				}
			}

			const std::vector<std::pair<uintptr_t, game::netadr_t>> oob_handlers =
			{
				{ 0xCE8B0C558B, *reinterpret_cast<game::netadr_t*>(rsp + 16 + 6 + 0x95 + 0x8) },
				{ 0xCE8B0C578B, *reinterpret_cast<game::netadr_t*>(rsp + 16 + 6 + 0x95) },
			};

			const auto oob = std::find_if(oob_handlers.begin(), oob_handlers.end(), [&](const auto& handler) { return (*reinterpret_cast<uint64_t*>(ret_address) & 0xFFFFFFFFFF) == handler.first; });

			if (oob != oob_handlers.end())
			{
				if (events::connectionless_packet::handle_command(oob->second, msg))
				{
					*msg = {};
				}
			}
			
			return value;
		}

		std::string build_lobby_msg(const game::LobbyModule module)
		{
			auto data{ ""s };
			const auto header{ 0x97Aui16 };
			data.append(reinterpret_cast<const char*>(&header), sizeof header);
			data.push_back(module);
			data.push_back(-1);
			return data;
		}

		void send_lobby_msg(const game::NetChanMsgType channel, const game::LobbyModule module, const game::msg_t& msg, const game::netadr_t& netadr, const std::uint64_t xuid)
		{
			auto data{ lobby_msg::build_lobby_msg(module) };
			data.append(reinterpret_cast<const char*>(msg.data), msg.cursize);

			constexpr auto interval = 75ms;
			const auto now = std::chrono::high_resolution_clock::now();
			static std::chrono::high_resolution_clock::time_point last_call{};
			static size_t count{ 0 };

			count = last_call + interval >= now ? count + 1 : 0;

			const auto send_lobby_msg = [=]()
			{
				game::net::netchan::send(channel, data, netadr, xuid);
			};

			scheduler::once(send_lobby_msg, scheduler::pipeline::main, interval * count);
			last_call = now;
		}
	}

	void send_lobby_msg(const game::LobbyModule module, const game::msg_t& msg, const game::netadr_t& netadr, const std::uint64_t xuid)
	{
		const auto session = game::session_data();

		if (session == nullptr)
			return;

		const auto channel = game::call<game::NetChanMsgType>(offsets::LobbyNetChan_GetLobbyChannel, session->type, game::LOBBY_CHANNEL_UNRELIABLE);
		return send_lobby_msg(channel, module, msg, netadr, xuid);
	}
	
	bool handle_packet(const game::LobbyModule module, const game::netadr_t& from, game::msg_t& msg)
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
		const auto biglong_ptr = utils::hook::scan_pattern(signatures::biglong_ptr);

		if (!biglong_ptr)
			return; 

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
			if (const auto biglong = utils::hook::extract<uintptr_t**>(utils::hook::extract<uint8_t*>(biglong_ptr + 1) + 3);
				biglong && *biglong != big_long_read_stub)
			{
				utils::hook::set(biglong, big_long_read_stub);
			}
		}, scheduler::pipeline::main);

		lobby_msg::on_message(game::LOBBY_MODULE_CLIENT, game::MESSAGE_TYPE_LOBBY_HOST_DISCONNECT_CLIENT, lobby_msg::handle_host_disconnect_client);
	}
}