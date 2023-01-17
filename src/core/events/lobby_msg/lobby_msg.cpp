#include "dependencies/stdafx.hpp"
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
	
	bool handle(const game::LobbyModule module, const game::netadr_t& from, game::msg_t& msg)
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
		lobby_msg::on_message(game::LOBBY_MODULE_CLIENT, game::MESSAGE_TYPE_LOBBY_HOST_DISCONNECT_CLIENT, lobby_msg::handle_host_disconnect_client);
	}
}