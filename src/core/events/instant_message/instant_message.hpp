#pragma once
#include "dependencies/std_include.hpp"

namespace events::instant_message
{
	void on_global_instant_message(void* thisptr, uint64_t sender_id, const char* sender_name, char* message, uint32_t message_size);
	void send_info_request(const std::vector<std::uint64_t>& recipients, const uint32_t nonce);
	void initialize();
	
	extern bool log_messages;
}