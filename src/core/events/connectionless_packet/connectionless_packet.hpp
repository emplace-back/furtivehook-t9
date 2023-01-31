#pragma once
#include "dependencies/stdafx.hpp"

namespace events::connectionless_packet
{
	bool handle_command(const game::netadr_t& from, game::msg_t* msg, bool server_oob = false);
	void initialize();
	extern bool log_commands;
}