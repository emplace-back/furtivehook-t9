#pragma once
#include "dependencies/std_include.hpp"

namespace events::instant_message
{
	void __fastcall on_global_instant_message(void* thisptr, uint64_t sender_id, const char* sender_name, char* message, uint32_t message_size);
	void initialize();
	
	extern bool log_messages;
}