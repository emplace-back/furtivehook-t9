#pragma once
#include <type_traits>
#include "dependencies/stdafx.hpp"

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
		MESSAGE_ELEMENT_DEBUG_START = 16,
		MESSAGE_ELEMENT_DEBUG_END = 17,
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
		int bit;
		int lastEntityRef;
		char pad[0x19];
		bool overflowed;
		bool readOnly;
		char pad2[0x8];
		MsgType type;
		PackageType packageType;
		char encodeFlags;

		msg_t()
		{
			::ZeroMemory(this, sizeof(msg_t) - 0x10);
			::memset(&this->pad, -1, 0x10);
			this->lastEntityRef = -1;
		}

		template<class T, const size_t bufsize>
		msg_t(T(&buf)[bufsize])
		{
			this->init(buf);
		}

		msg_t(void* buf, const size_t bufsize, const bool read_only)
		{
			this->init(buf, bufsize, read_only);
		}
		
		auto get_data() const
		{
			return std::string{ reinterpret_cast<const char*>(this->data), static_cast<std::string::size_type>(this->cursize) };
		}
		
		void init(void* buf, const size_t bufsize, const bool read_only = false)
		{
			*this = {};

			this->data = reinterpret_cast<char*>(buf);
			this->maxsize = static_cast<int>(bufsize);

			if (read_only)
			{
				this->cursize = static_cast<int>(bufsize);
				this->readOnly = true;
			}
		}

		template<class T, const size_t bufsize>
		void init(T(&buf)[bufsize])
		{
			this->init(buf, bufsize);
		}

		template<class T, const size_t bufsize>
		bool init_lobby_write(T(&buf)[bufsize], const MsgType msg_type)
		{
			this->init(buf);

			this->packageType = PACKAGE_TYPE_WRITE;
			this->type = msg_type;
			this->encodeFlags = 0;

			this->write(0, 2);

			this->write<uint8_t>(MESSAGE_ELEMENT_UINT8);
			this->write<uint8_t>(msg_type);

			if (this->overflowed)
			{
				return false;
			}

			this->write<uint8_t>(MESSAGE_ELEMENT_STRING);
			const auto msg_type_name = LobbyTypes_GetMsgTypeName(static_cast<MsgType>(-1)); 
			this->write(msg_type_name);
			
			return true;
		}

		bool init_lobby_read();
		
		template<typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
		void write(const T value)
		{
			const auto cur_size = static_cast<size_t>(this->cursize);
			const auto max_size = static_cast<size_t>(this->maxsize);

			if (cur_size + sizeof(value) > max_size)
			{
				this->overflowed = true;
			}
			else
			{
				*reinterpret_cast<T*>(&this->data[cur_size]) = value;
				this->cursize += sizeof(value);
			}
		}

		void write(int value, int bits)
		{
			const auto cur_size = static_cast<size_t>(this->cursize);
			const auto max_size = static_cast<size_t>(this->maxsize);

			if (max_size - cur_size < 4)
			{
				this->overflowed = true;
			}
			else
			{
				while (bits)
				{
					--bits;

					const auto bit = this->bit & 7;

					if (!bit)
					{
						this->bit = sizeof(uint64_t) * cur_size;
						this->data[++this->cursize] = 0;
					}

					if ((value & 1) != 0)
						this->data[this->bit >> 3] |= 1 << bit;

					++this->bit;
					value >>= 1;
				}
			}
		}

		void write(const void* buf, const size_t bufsize)
		{
			const auto cur_size = static_cast<size_t>(this->cursize);
			const auto max_size = static_cast<size_t>(this->maxsize);

			if (cur_size + bufsize > max_size)
			{
				this->overflowed = true;
			}
			else
			{
				std::memcpy(&this->data[cur_size], buf, bufsize);
				this->cursize += bufsize;
			}
		}

		void write(const std::string& buffer)
		{
			return write(buffer.data(), buffer.size() + 1);
		}

		template<typename T>
		void write_lobby(const T value, const uint8_t element_type)
		{
			if (packageType != PACKAGE_TYPE_WRITE)
				return;

			this->write<uint8_t>(element_type);
			this->write<T>(value);
		}

		template<typename T> 
		T read()
		{
			const auto cur_size = static_cast<size_t>(this->cursize);
			const auto read_count = static_cast<size_t>(this->readcount); 

			T result = {};

			if (read_count + sizeof(T) > cur_size)
			{
				this->overflowed = true;
			}
			else
			{
				result = *reinterpret_cast<T*>(&this->data[read_count]);
				this->readcount += sizeof(T);
			}

			return result;
		}

		template<typename T>
		T read(const T bits)
		{
			const auto cur_size = static_cast<size_t>(this->cursize);
			const auto read_count = static_cast<size_t>(this->readcount); 
			
			T result = {};

			for (auto i = 0; i < bits; ++i)
			{
				const auto bit = this->bit & 7;
				if (!bit)
				{
					if (read_count >= cur_size)
					{
						this->overflowed = true;
						return -1;
					}

					this->bit = sizeof(uint64_t) * ++this->readcount;
				}

				result |= ((this->data[++this->bit >> 3] >> bit) & 1) << i;
			}

			return result;
		}

		void read(void* buf, const size_t bufsize, const size_t readlen)
		{
			const auto cur_size = static_cast<size_t>(this->cursize);
			const auto read_count = static_cast<size_t>(this->readcount);
			
			if (readlen > bufsize || read_count + readlen > cur_size)
			{
				this->overflowed = true;
				std::memset(buf, -1, bufsize);
			}
			else
			{
				std::memcpy(buf, &this->data[read_count], readlen);
				this->readcount += readlen;
			}
		}

		template<class T, const size_t bufsize>
		void read(T(&buf)[bufsize], const size_t readlen)
		{
			this->read(buf, bufsize, readlen);
		}

		void read_string(char* str, const size_t strsize, const bool next = false)
		{
			for (auto l = 0; ; ++l)
			{
				auto c = read<uint8_t>(); 
				
				if (l < strsize)
				{
					if (next && c == '\n' || c == -1)
						c = 0;
					
					if (c == '%')
						c = '.'; 
					
					str[l] = c;
				}
				
				if (!c)
					break;
			}
			
			str[strsize - 1] = 0;
		}

		template<class T, const size_t strsize>
		void read_string(T(&str)[strsize], const bool next = false)
		{
			this->read_string(str, strsize, next);
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
		char pad[0xE0];
		bool inputDevice;
		char pad2[0x26F];
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
