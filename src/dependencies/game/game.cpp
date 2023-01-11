#include "dependencies/std_include.hpp"
#include "game.hpp"

namespace game
{
	namespace net
	{
		namespace netchan
		{
			bool send(const NetChanMsgType type, const std::string& data, const netadr_t& netadr, const uint64_t xuid)
			{
				return game::call<bool>(offsets::Netchan_SendMessage, 0, type, NETCHAN_UNRELIABLE, data.data(), data.size(), xuid, netadr, nullptr);
			}
		}
		
		namespace oob
		{
			bool send(const netadr_t& netadr, const std::string& data)
			{
				return game::call<bool>(offsets::NET_OutOfBandData, game::NS_SERVER, netadr, data.data(), data.size());
			}
		}
	}
	
	void initialize()
	{
		exception::initialize();
		rendering::initialize();
		events::initialize();

		PRINT_LOG("Initialized!");
	}

	uintptr_t get_base()
	{
		static auto base{ utils::nt::library{}.get_ptr() };
		return reinterpret_cast<uintptr_t>(base);
	}

	void send_instant_message(const std::vector<std::uint64_t>& recipients, const std::uint8_t type, const void* message, const uint32_t message_size)
	{
		const auto lobby = game::dwGetLobby(0);

		if (!lobby)
			return;

		const auto self = std::find_if(recipients.begin(), recipients.end(), [=](const auto& id) { return id == LiveUser_GetXuid(0); });

		if (self != recipients.end())
		{
			char buffer[0x1000] = { 0 };
			msg_t msg{};

			msg.init(buffer, sizeof(buffer));
			msg.write<uint8_t>('2');
			msg.write<uint8_t>(type);
			msg.write_data(reinterpret_cast<const char*>(message), message_size);

			return events::instant_message::on_global_instant_message(
				*reinterpret_cast<uintptr_t**>(lobby + 0x750),
				*self,
				utils::string::va("%llx", *self).data(),
				msg.data,
				msg.cursize);
		}

		return call(offsets::dwInstantSendMessage, 0, recipients.data(), recipients.size(), type, message, message_size);
	}

	void send_instant_message(const std::vector<std::uint64_t>& recipients, const std::uint8_t type, const std::string& data)
	{
		return send_instant_message(recipients, type, data.data(), data.size());
	}

	void send_instant_message(const std::vector<std::uint64_t>& recipients, const std::uint8_t type, const msg_t& msg)
	{
		return send_instant_message(recipients, type, msg.data, msg.cursize);
	}
	
	const char* LobbyTypes_GetMsgTypeName(const MsgType type)
	{
		if (type < MESSAGE_TYPE_INFO_REQUEST || type >= MESSAGE_TYPE_COUNT)
		{
			return "Invalid";
		}

		return reinterpret_cast<const char**>(OFFSET(offsets::s_lobbyMsgName))[type];
	}

	int find_target_from_addr(const LobbySession* session, const netadr_t& from)
	{
		if (session == nullptr)
			return -1;

		for (size_t i = 0; i < 18; ++i)
		{
			if (const auto client = session->clients[i].activeClient; client)
			{
				const auto netadr = client->sessionInfo[session->type].netAdr;
				if (from == netadr)
					return i;
			}
		}

		return -1;
	}

	void Cbuf_AddText(const char* text)
	{
		struct CmdText
		{
			char data[0x1F400];
			int maxsize;
			int cmdsize;
		};

		const auto cmd_text_buf = reinterpret_cast<CmdText*>(OFFSET(offsets::cmd_text_buf));
		const auto length = std::strlen(text);

		if (!cmd_text_buf)
			return;

		if (cmd_text_buf->cmdsize + length >= cmd_text_buf->maxsize)
			return;

		std::memcpy(&cmd_text_buf->data[cmd_text_buf->cmdsize], text, length + 1);
		cmd_text_buf->cmdsize += length;
	}
}