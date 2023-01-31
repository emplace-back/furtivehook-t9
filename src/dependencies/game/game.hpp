#pragma once
#include "utils/string.hpp"
#include "structs.hpp"
#include "offsets.hpp"
#include "signatures.hpp"

#define OFFSET(x) game::relocate(x)
#define FUNC(NAME, TYPE) const static auto NAME = reinterpret_cast<std::add_pointer<TYPE>::type>(OFFSET(offsets::NAME))

namespace game 
{
	namespace net
	{
		namespace netchan
		{
			bool get(const NetChanMessage_s * chan, msg_t* msg, NetChanMsgType type); 
			bool send(const NetChanMsgType type, const std::string& data, const netadr_t& netadr, const uint64_t xuid);
		}
		
		namespace oob
		{
			netadr_t register_remote_addr(const HostInfo& host_info);
			netadr_t register_remote_addr(const InfoResponseLobby& lobby);
			bool send(const netadr_t& target, const std::string& data);
		}
	}

	void initialize();
	uintptr_t get_base();
	TLSData* Sys_GetTLS();
	void send_instant_message(const std::vector<uint64_t>& recipients, const uint8_t type, const void* message, const uint32_t message_size);
	void send_instant_message(const std::vector<uint64_t>& recipients, const uint8_t type, const std::string& data);
	void send_instant_message(const std::vector<uint64_t>& recipients, const uint8_t type, const msg_t& msg);
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
		return utils::spoof_call(reinterpret_cast<T(*)(Args...)>(OFFSET(address)), args...);
	}

	FUNC(LiveUser_GetXuid, uint64_t(int));
	FUNC(Live_IsUserSignedInToDemonware, bool(int));
	FUNC(dwGetLobby, uintptr_t(int));
	FUNC(dwNetadrToCommonAddr, bool(netadr_t, void*, const uint32_t, void*));
}
