#pragma once
#include "dependencies/stdafx.hpp"
#include "utils/string.hpp"
#include "utils/spoof_call/spoof_call.hpp"
#include "structs.hpp"
#include "offsets.hpp"
#include "signatures.hpp"

#define OFFSET(x) game::relocate(x)

namespace game 
{
	namespace net
	{
		namespace netchan
		{
			bool send(const NetChanMsgType type, const std::string& data, const netadr_t& netadr, const uint64_t xuid);
		}
		
		namespace oob
		{
			game::netadr_t register_remote_addr(const game::HostInfo& host_info);
			game::netadr_t register_remote_addr(const game::InfoResponseLobby& lobby);
			bool send(const netadr_t& target, const std::string& data);
		}
	}

	void initialize();
	uintptr_t get_base();
	void send_instant_message(const std::vector<std::uint64_t>& recipients, const std::uint8_t type, const void* message, const uint32_t message_size);
	void send_instant_message(const std::vector<std::uint64_t>& recipients, const std::uint8_t type, const std::string& data);
	void send_instant_message(const std::vector<std::uint64_t>& recipients, const std::uint8_t type, const msg_t& msg);
	const char* LobbyTypes_GetMsgTypeName(const MsgType type);
	int find_target_from_addr(const LobbySession* session, const netadr_t& from);
	void Cbuf_AddText(const char* text);

	extern CmdText* cmd_text;
	
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
		const auto controlling_session = get_client_session(LOBBY_TYPE_GAME)->active > SESSION_INACTIVE;
		return get_client_session(controlling_session);
	}
	
	template <typename T = void, typename... Args>
	inline auto call(const uintptr_t address, Args ... args)
	{
		return spoof_call(reinterpret_cast<T(*)(Args...)>(relocate(address)), args...);
	}

	const static auto LiveUser_GetXuid = reinterpret_cast<uint64_t(*)(int)>(OFFSET(offsets::Live_GetXuid));
	const static auto Live_IsUserSignedInToDemonware = reinterpret_cast<bool(*)(int)>(OFFSET(offsets::Live_IsUserSignedInToDemonware));
	const static auto dwGetLobby = reinterpret_cast<uintptr_t(*)(int)>(OFFSET(offsets::dwGetLobby));
	const static auto dwNetadrToCommonAddr = reinterpret_cast<bool(*)(netadr_t, void*, const uint32_t, void*)>(OFFSET(offsets::dwNetadrToCommonAddr));
	const static auto Sys_GetTLS = reinterpret_cast<TLSData*(*)()>(OFFSET(offsets::Sys_GetTLS));
}
