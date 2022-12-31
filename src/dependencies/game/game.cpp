#include "dependencies/std_include.hpp"
#include "game.hpp"

namespace game
{
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
		const auto self = std::find_if(recipients.begin(), recipients.end(), [=](const auto& id) { return id == LiveUser_GetXuid(0); });

		if(self == recipients.end())
			return call(offsets::dwInstantSendMessage, 0, recipients.data(), recipients.size(), type, message, message_size);

		if (const auto lobby = call<uintptr_t>(offsets::dwGetLobby, 0))
		{
			char buffer[0x1000] = { 0 };
			msg_t msg{};

			msg.init(buffer, sizeof(buffer));
			msg.write<uint8_t>('2');
			msg.write<uint8_t>(type);
			msg.write_data(reinterpret_cast<const char*>(message), message_size);

			return events::instant_message::on_global_instant_message(*reinterpret_cast<uintptr_t**>(lobby + 0x750), *self, "zaddy", msg.data, msg.cursize);
		}
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
			if (const auto client = session->get_client(i).activeClient; client)
			{
				const auto netadr = client->sessionInfo[session->type].netAdr;
				if (from == netadr)
					return i;
			}
		}

		return -1;
	}

	void Cmd_ExecuteSingleCommand(const std::string& data)
	{
		char buffer[0x1000] = { 0 };
		game::msg_t msg{};

		msg.init(buffer, sizeof(buffer));
		msg.write_data(data);

		return game::call(offsets::dwInstantHandleRemoteCommand, 0, 0, &msg);
	}
}