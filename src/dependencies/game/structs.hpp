#pragma once
#include "dependencies/std_include.hpp"

namespace game 
{
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
		LOBBY_MODULE_PEER_TO_PEER,
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
		char pad[0x21];
		bool overflowed;
		bool readOnly;
		char pad2[0x8];
		MsgType type;
		PackageType packageType;
		char encodeFlags;

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
	};

	struct netadr_t
	{
		union
		{
			uint8_t ip[4];
			uint32_t inaddr;
		};

		std::uint16_t port;
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

		const std::string to_string(bool port = false) const
		{
			return port ? utils::string::va("%u.%u.%u.%u:%u", addr[0], addr[1], addr[2], addr[3], port) : utils::string::va("%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);
		}
	};
#pragma pack(pop)
	
	struct SessionInfo
	{
		bool inSession;
		netadr_t netAdr;
		time_t lastMessageSentToPeer;
	}; 
	
	struct FixedClientInfo
	{
		char pad[0x8];
		uint64_t xuid;
		char pad2[0xB5];
		char gamertag[36];
	}; 
	
	struct ActiveClient
	{
		char pad[0x358];
		FixedClientInfo fixedClientInfo;
		char pad2[0x60];
		SessionInfo sessionInfo[3];
	};

	struct SessionClient
	{
		char pad[0x10];
		uint64_t xuid;
		ActiveClient* activeClient;
		char pad2[0x20];
	};

	struct LobbySession
	{
		LobbyModule module;
		LobbyType type;
		int mode;
		char pad[0x34];
		SessionActive active;
		char pad2[0x148];
		SessionClient clients[18];
		char pad3[0x11FC0];
	};

	struct PresenceData
	{
		char pad[0x360];
	};
}
