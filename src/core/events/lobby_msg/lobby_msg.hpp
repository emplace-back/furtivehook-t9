#pragma once

namespace events::lobby_msg
{
	void send_lobby_msg(const game::LobbyModule module, const game::msg_t& msg, const game::netadr_t& netadr, const std::uint64_t xuid);
	bool handle(const game::LobbyModule module, const game::netadr_t& from, game::msg_t& msg);
	void initialize();
	extern bool log_messages;

	template <typename T>
	bool handle(const T value, const char* key)
	{
		const std::vector<std::pair<std::string, T>> patches =
		{
			{ "lobbytype", 2 },
			{ "clientcount", 18 },
		};

		return std::any_of(patches.begin(), patches.end(), [=](const auto& p) { return p.first == key && static_cast<T>(value) > p.second; });
	}
}