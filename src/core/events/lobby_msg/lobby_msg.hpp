#pragma once

namespace events::lobby_msg
{
	void send_lobby_msg(const game::LobbyModule module, const game::msg_t& msg, const game::netadr_t& netadr, const std::uint64_t xuid);
	bool handle_packet(const game::LobbyModule module, const game::netadr_t& from, game::msg_t& msg);
	void initialize();
	extern bool log_messages;
}