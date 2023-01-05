#pragma once

namespace events::lobby_msg
{
	bool __fastcall handle_packet(const game::LobbyModule module, const game::netadr_t& from, game::msg_t& msg);
	void initialize();
	extern bool log_messages;
}