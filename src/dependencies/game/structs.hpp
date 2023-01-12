#pragma once
#include <type_traits>
#include "dependencies/std_include.hpp"

namespace game 
{
	enum NetchanMsgMode
	{
		NETCHAN_RELIABLE,
		NETCHAN_UNRELIABLE,
	}; 
	
	enum NetChanMsgType
	{
		NETCHAN_INVALID_CHANNEL = -1,
		NETCHAN_SNAPSHOT,
		NETCHAN_CLIENTMSG,
		NETCHAN_VOICE,
		NETCHAN_LOBBY_VOICE,
		NETCHAN_LOBBYPRIVATE_STATE,
		NETCHAN_LOBBYPRIVATE_HEARTBEAT,
		NETCHAN_LOBBYPRIVATE_RELIABLE,
		NETCHAN_LOBBYPRIVATE_UNRELIABLE,
		NETCHAN_LOBBYPRIVATE_MIGRATE,
		NETCHAN_LOBBYGAME_STATE,
		NETCHAN_LOBBYGAME_HEARTBEAT,
		NETCHAN_LOBBYGAME_RELIABLE,
		NETCHAN_LOBBYGAME_UNRELIABLE,
		NETCHAN_LOBBYGAME_MIGRATE,
		NETCHAN_LOBBYTRANSITION_STATE,
		NETCHAN_LOBBYTRANSITION_HEARTBEAT,
		NETCHAN_LOBBYTRANSITION_RELIABLE,
		NETCHAN_LOBBYTRANSITION_UNRELIABLE,
		NETCHAN_LOBBYTRANSITION_MIGRATE,
		NETCHAN_LOBBY_JOIN,
		NETCHAN_PTP,
		NETCHAN_CLIENT_CONTENT,
		NETCHAN_CLIENT_CMD,
		NETCHAN_CONNECTIONLESS_CMD,
		NETCHAN_TEST,
		NETCHAN_MAX_CHANNELS,
	};

	enum LobbyDisconnectClient
	{
		LOBBY_DISCONNECT_CLIENT_INVALID = -1,
		LOBBY_DISCONNECT_CLIENT_DROP,
		LOBBY_DISCONNECT_CLIENT_KICK,
		LOBBY_DISCONNECT_CLIENT_BADDLC,
		LOBBY_DISCONNECT_CLIENT_KICK_PARTY,
		LOBBY_DISCONNECT_CLIENT_HOSTRELOAD,
		LOBBY_DISCONNECT_CLIENT_NOPARTYCHAT,
	}; 
	
	enum LobbyChannel
	{
		LOBBY_CHANNEL_HEARTBEAT,
		LOBBY_CHANNEL_STATE,
		LOBBY_CHANNEL_UNRELIABLE,
		LOBBY_CHANNEL_RELIABLE,
		LOBBY_CHANNEL_MIGRATE,
		LOBBY_CHANNEL_PEER_TO_PEER,
		LOBBY_CHANNEL_COUNT,
	};
	
	enum netsrc_t
	{
		NS_NULL = -1,
		NS_CLIENT1,
		NS_CLIENT2,
		NS_CLIENT3,
		NS_CLIENT4,
		NS_SERVER,
		NS_MAXCLIENTS,
		NS_PACKET,
	}; 
	
	enum netadrtype_t
	{
		NA_BOT,
		NA_BAD,
		NA_LOOPBACK,
		NA_RAWIP,
		NA_IP,
	}; 

	enum LobbyMsgElementType
	{
		MESSAGE_ELEMENT_INT32,
		MESSAGE_ELEMENT_UINT32,
		MESSAGE_ELEMENT_UINT8 = 5,
		MESSAGE_ELEMENT_XUID = 9,
		MESSAGE_ELEMENT_STRING = 11,
		MESSAGE_ELEMENT_GLOB = 12,
	};
	
	enum PackageType
	{
		PACKAGE_TYPE_NONE,
		PACKAGE_TYPE_WRITE,
		PACKAGE_TYPE_READ,
	}; 
	
	enum IMType
	{
		JOIN_REQUEST,
		JOIN_REPLY,
	}; 
	
	enum MsgType
	{
		MESSAGE_TYPE_NONE = -1,
		MESSAGE_TYPE_INFO_REQUEST,
		MESSAGE_TYPE_INFO_RESPONSE,
		MESSAGE_TYPE_LOBBY_STATE_PRIVATE,
		MESSAGE_TYPE_LOBBY_STATE_GAME,
		MESSAGE_TYPE_LOBBY_STATE_TRANSITION,
		MESSAGE_TYPE_LOBBY_HOST_HEARTBEAT,
		MESSAGE_TYPE_LOBBY_HOST_DISCONNECT,
		MESSAGE_TYPE_LOBBY_HOST_DISCONNECT_CLIENT,
		MESSAGE_TYPE_LOBBY_HOST_LEAVE_WITH_PARTY,
		MESSAGE_TYPE_LOBBY_HOST_LOBBY_MOVE,
		MESSAGE_TYPE_LOBBY_CLIENT_HEARTBEAT,
		MESSAGE_TYPE_LOBBY_CLIENT_DISCONNECT,
		MESSAGE_TYPE_LOBBY_CLIENT_RELIABLE_DATA,
		MESSAGE_TYPE_LOBBY_CLIENT_CONTENT,
		MESSAGE_TYPE_LOBBY_MODIFIED_LOADOUT,
		MESSAGE_TYPE_LOBBY_MODIFIED_STATS,
		MESSAGE_TYPE_JOIN_LOBBY,
		MESSAGE_TYPE_JOIN_RESPONSE,
		MESSAGE_TYPE_JOIN_AGREEMENT_REQUEST,
		MESSAGE_TYPE_JOIN_AGREEMENT_RESPONSE,
		MESSAGE_TYPE_JOIN_COMPLETE,
		MESSAGE_TYPE_JOIN_MEMBER_INFO,
		MESSAGE_TYPE_SERVERLIST_INFO,
		MESSAGE_TYPE_PEER_TO_PEER_CONNECTIVITY_TEST,
		MESSAGE_TYPE_PEER_TO_PEER_INFO,
		MESSAGE_TYPE_BANDWIDTH_CONNECTIVITY_TEST,
		MESSAGE_TYPE_LOBBY_MIGRATE_ANNOUNCE_HOST,
		MESSAGE_TYPE_LOBBY_MIGRATE_START,
		MESSAGE_TYPE_INGAME_MIGRATE_TO,
		MESSAGE_TYPE_INGAME_MIGRATE_NEW_HOST,
		MESSAGE_TYPE_VOICE_PACKET,
		MESSAGE_TYPE_VOICE_RELAY_PACKET,
		MESSAGE_TYPE_LUA_MESSAGE,
		MESSAGE_TYPE_CHAT_MESSAGE,
		MESSAGE_TYPE_COUNT,
	};

	enum LobbyModule
	{
		LOBBY_MODULE_INVALID = -1,
		LOBBY_MODULE_HOST,
		LOBBY_MODULE_CLIENT,
		LOBBY_MODULE_PEER_TO_PEER = 3,
		LOBBY_MODULE_LUA_HOST,
		LOBBY_MODULE_LUA_CLIENT,
		LOBBY_MODULE_COUNT,
	};

	enum LobbyType
	{
		LOBBY_TYPE_INVALID = -1,
		LOBBY_TYPE_PRIVATE,
		LOBBY_TYPE_GAME,
		LOBBY_TYPE_TRANSITION,
		LOBBY_TYPE_COUNT,
	};

	enum SessionActive
	{
		SESSION_INACTIVE,
		SESSION_KEEP_ALIVE,
		SESSION_ACTIVE,
	};
	
	struct JoinSessionMessage
	{
		IMType mType;
		char pad[0x84];
	}; 
	
	struct msg_t
	{
		char* data;
		int maxsize;
		int cursize;
		int readcount;
		char pad[0x18];
		int bit;
		char pad2[0x5];
		bool overflowed;
		bool readOnly;
		char pad3[0x8];
		MsgType type;
		PackageType packageType;
		char encodeFlags;

		template<size_t buf_size>
		void init_lobby(char(&buf)[buf_size], const MsgType msg_type)
		{
			this->init(buf, buf_size);

			this->packageType = PACKAGE_TYPE_WRITE;
			this->type = msg_type;
			this->encodeFlags = 0;

			this->write_bits(0, 2);
			this->write<uint8_t>(MESSAGE_ELEMENT_UINT8);
			this->write<uint8_t>(msg_type);
			this->write<uint8_t>(MESSAGE_ELEMENT_STRING);
			this->write_data("sike");
		}
		
		void init(char* buffer, const size_t buf_size, const bool read = false)
		{
			*this = {};

			data = buffer;
			maxsize = buf_size;

			if (readOnly = read)
			{
				cursize = buf_size;
			}
		}
		
		template<typename T> void write_lobby(T value, const uint8_t element_type)
		{
			if (packageType != PACKAGE_TYPE_WRITE)
				return;
			
			this->write<uint8_t>(element_type);
			this->write<T>(value);
		}

		void write_data_lobby(const char* data, const size_t length)
		{
			if (packageType != PACKAGE_TYPE_WRITE)
				return;

			this->write<uint8_t>(MESSAGE_ELEMENT_GLOB);
			this->write<uint16_t>(length);
			this->write_data(data, length);
		}
		
		template<typename T> void write(T value)
		{
			const auto final_size = cursize + sizeof(value);

			if (final_size > maxsize)
			{
				overflowed = 1;
			}
			else
			{
				*reinterpret_cast<T*>(&data[cursize]) = value;
				cursize = final_size;
			}
		}
		
		void write_data(const std::string& buffer)
		{
			return write_data(buffer.data(), buffer.size() + 1);
		}

		void write_data(const char* buffer, const size_t length)
		{
			const auto final_size = cursize + length;

			if (final_size > maxsize)
			{
				overflowed = 1;
			}
			else
			{
				std::memcpy(&data[cursize], buffer, length);
				cursize = final_size;
			}
		}

		void write_bits(int value, int bits)
		{
			if (static_cast<uint32_t>(bits) > 0x20)
				return;

			if (maxsize - cursize < 4)
			{
				overflowed = 1;
			}
			else
			{
				while (bits)
				{
					--bits;

					const auto b = bit & 7;

					if (!b)
					{
						bit = sizeof(uint64_t) * cursize;
						data[++cursize] = 0;
					}

					if ((value & 1) != 0)
						data[bit >> 3] |= 1 << b;

					++bit;
					value >>= 1;
				}
			}
		}

		template<typename T> T read()
		{
			const auto final_size = readcount + sizeof(T);
			auto result = static_cast<T>(-1);

			if (final_size > cursize)
			{
				overflowed = 1;
			}
			else
			{
				result = *reinterpret_cast<T*>(&data[readcount]);
				readcount = final_size;
			}

			return result;
		}

		void read_data(void* buffer, const size_t max_size, const size_t length)
		{
			if (length > max_size || readcount + length > cursize)
			{
				overflowed = 1;
				std::memset(data, 0xff, max_size);
			}
			else
			{
				std::memcpy(buffer, &data[readcount], length);
				readcount += length;
			}
		}
	};

	struct netadr_t
	{
		union
		{
			uint8_t ip[4];
			uint32_t inaddr;
		};

		uint16_t port;
		netadrtype_t type;
		netsrc_t localNetID;
	};

#pragma pack(push, 1)
	struct XNADDR
	{
		char pad[0x1E];
		union
		{
			uint8_t addr[4];
			uint32_t inaddr;
		};

		uint16_t port;
		char pad2[0x30];

		const std::string to_string(bool include_port = false) const
		{
			return include_port ? utils::string::va("%u.%u.%u.%u:%u", addr[0], addr[1], addr[2], addr[3], include_port) : utils::string::va("%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);
		}
	};
#pragma pack(pop)
	
#pragma pack(push, 1)
	struct bdSecurityID
	{
		uint64_t id;
	};
#pragma pack(pop)

	struct bdSecurityKey
	{
		char ab[16];
	}; 
	
	struct XSESSION_INFO
	{
		bdSecurityID sessionID;
		XNADDR hostAddress;
		bdSecurityKey keyExchangeKey;
	}; 
	
	struct SerializedAdr
	{
		bool valid;
		XNADDR xnaddr;
	};

	struct LobbyParams
	{
		int networkMode;
		int mainMode;
	};

	struct InfoResponseLobby
	{
		bool isValid;
		uint64_t hostXuid;
		char hostName[36];
		bdSecurityID secId;
		bdSecurityKey secKey;
		SerializedAdr serializedAdr;
		int status;
		LobbyParams lobbyParams;

		game::XSESSION_INFO get_sess_info() const
		{
			if (!isValid)
				return {};

			game::XSESSION_INFO info{};
			info.sessionID = secId;
			info.keyExchangeKey = secKey;
			info.hostAddress = serializedAdr.xnaddr;
			return info;
		}
	};

	struct Msg_InfoResponse
	{
		uint32_t nonce;
		int uiScreen;
		int playlistID;
		uint8_t natType;
		InfoResponseLobby lobby[3];
	};

	struct SessionInfo
	{
		bool inSession;
		netadr_t netAdr;
		time_t lastMessageSentToPeer;
	}; 
	
	struct FixedClientInfo
	{
		int platform;
		int dwPlatformType;
		uint64_t xuid;
		char pad2[0xB5];
		char gamertag[36];
		char name[64];
		char platformName[36];
	};

	struct MutableClientInfo
	{
		char pad[0x350];
	};
	
	struct ActiveClient
	{
		MutableClientInfo mutableClientInfo; 
		char pad[0x8];
		FixedClientInfo fixedClientInfo;
		SessionInfo sessionInfo[3];
	};

	struct SessionClient
	{
		char pad[0x10];
		uint64_t xuid;
		ActiveClient* activeClient;
		char pad2[0x20];
	};

	struct HostInfo
	{
		uint64_t xuid;
		char name[36];
		char pad[0x24];
		netadr_t netadr;
		SerializedAdr serializedAdr;
		bdSecurityID secId;
		bdSecurityKey secKey;
		uint32_t serverLocation;

		game::HostInfo from_lobby(const game::InfoResponseLobby& lobby)
		{
			game::HostInfo host_info{};
			host_info.xuid = lobby.hostXuid;
			const auto sess_info = lobby.get_sess_info();
			host_info.secId = sess_info.sessionID;
			host_info.secKey = sess_info.keyExchangeKey;
			host_info.serializedAdr.xnaddr = sess_info.hostAddress;
			return host_info;
		}
	};

	struct SessionHost
	{
		HostInfo info;
	};

	struct LobbySession
	{
		LobbyModule module;
		LobbyType type;
		int mode;
		char pad[0x38];
		SessionActive active;
		char pad2[0x5C];
		SessionHost host;
		char pad3[0xA];
		SessionClient clients[18];
		char pad4[0x11FC0];
	};

	struct CmdArgs
	{
		int nesting;
		int localClientNum[8];
		int controllerIndex[8];
		int argshift[8];
		int argc[8];
		const char** argv[8];
		char textPool[8192];
		const char* argvPool[512];
		int usedTextPool[8];
		int totalUsedArgvPool;
		int totalUsedTextPool;
	};

	struct TLSData
	{
		void* vaInfo;
		jmp_buf* errorJmpBuf;
		void* traceInfo;
		CmdArgs* cmdArgs;
		void* errorData;
	};

	struct CmdText
	{
		char data[0x1F400];
		int maxsize;
		int cmdsize;
	};
}
