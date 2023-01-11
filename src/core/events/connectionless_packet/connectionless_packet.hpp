#pragma once
#include "dependencies/std_include.hpp"

namespace events::connectionless_packet
{
	bool handle_command(const game::netadr_t& from, game::msg_t* msg, const bool header = true);
	void initialize();
	extern bool log_commands;
}