#pragma once
#include "dependencies/std_include.hpp"
#include "utils/string.hpp"
#include "utils/spoof_call/spoof_call.hpp"
#include "structs.hpp"
#include "offsets.hpp"

#define OFFSET(x) game::relocate(x)

namespace game 
{
	void initialize();
	uintptr_t get_base();
	void send_instant_message(const std::vector<std::uint64_t>& recipients, const std::uint8_t type, const void* message, const uint32_t message_size);
	void send_instant_message(const std::vector<std::uint64_t>& recipients, const std::uint8_t type, const std::string& data);
	void send_instant_message(const std::vector<std::uint64_t>& recipients, const std::uint8_t type, const msg_t& msg);
	const char* LobbyTypes_GetMsgTypeName(const MsgType type);
	int find_target_from_addr(const LobbySession* session, const netadr_t& from);
	void Cbuf_AddText(const char* text);
	
	inline uintptr_t relocate(const uintptr_t val)
	{
		const auto base = get_base();
		return base + (val - offsets::dump_base);
	}

	inline uintptr_t derelocate(const uintptr_t val)
	{
		const auto base = get_base();
		return (val - base) + offsets::dump_base;
	}

	inline uintptr_t derelocate(const void* val)
	{
		return derelocate(reinterpret_cast<size_t>(val));
	}

	inline LobbySession* get_client_session(const int lobby_type)
	{
		return reinterpret_cast<LobbySession*>(OFFSET(offsets::s_clientSession) + sizeof(LobbySession) * lobby_type);
	}

	inline LobbySession* session_data()
	{
		const auto party_session = game::get_client_session(game::LOBBY_TYPE_PRIVATE);
		const auto lobby_session = game::get_client_session(game::LOBBY_TYPE_GAME);

		return lobby_session->active ? lobby_session : party_session;
	}
	
	template <typename T = void, typename... Args>
	inline auto call(const uintptr_t address, Args ... args)
	{
		return spoof_call(reinterpret_cast<T(*)(Args...)>(relocate(address)), args...);
	}

	const static auto CL_AddReliableCommand = reinterpret_cast<void(*)(int, const char*)>(OFFSET(offsets::CL_AddReliableCommand));
	const static auto LiveUser_GetXuid = reinterpret_cast<uint64_t(*)(int)>(OFFSET(offsets::Live_GetXuid));
	const static auto Live_IsUserSignedInToDemonware = reinterpret_cast<bool(*)(int)>(OFFSET(offsets::Live_IsUserSignedInToDemonware));
	const static auto dwNetadrToCommonAddr = reinterpret_cast<bool(*)(netadr_t, void*, const uint32_t, void*)>(OFFSET(offsets::dwNetadrToCommonAddr));
}
