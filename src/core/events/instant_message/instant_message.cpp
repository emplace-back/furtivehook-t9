#include "dependencies/std_include.hpp"
#include "instant_message.hpp"

namespace events::instant_message
{
	utils::hook::vmt lobby_event_handlers_vmt{ 75 }; 
	bool log_messages = true;

	namespace
	{
		using callback = std::function<bool(game::msg_t&, const char*, const std::uint64_t)>;
		
		std::unordered_map<std::uint8_t, callback>& get_callbacks()
		{
			static std::unordered_map<std::uint8_t, callback> callbacks{};
			return callbacks;
		}
		
		void on_message(const std::uint8_t type, const callback& callback)
		{
			get_callbacks()[type] = callback;
		}

		bool dispatch_message(const char* sender_name, const std::uint64_t sender_id, char* message, const uint32_t message_size)
		{
			game::msg_t msg{};
			msg.init(message, message_size, true);

			auto type{ 0ui8 };

			if (msg.read<uint8_t>() == '2')
				type = msg.read<uint8_t>();

			if (log_messages)
				PRINT_LOG("Received instant message '%c' of size [%u] from '%s' (%llu)", type, message_size, sender_name, sender_id);

			const auto& callbacks = get_callbacks();
			const auto handler = callbacks.find(type);

			if (handler == callbacks.end())
				return false;

			return handler->second(msg, sender_name, sender_id);
		}
	}

	void __fastcall on_global_instant_message(void* thisptr, uint64_t sender_id, const char* sender_name, char* message, uint32_t message_size)
	{
		if (!instant_message::dispatch_message(sender_name, sender_id, message, message_size))
		{
			return lobby_event_handlers_vmt.call(27, thisptr, sender_id, sender_name, message, message_size);
		}
	}
	
	void initialize()
	{
		instant_message::on_message('e', [=](auto& msg, const auto& sender_name, const auto& sender_id)
		{
			PRINT_MESSAGE("Instant Message", "Remote command attempt caught from '%s' (%llu)", sender_name, sender_id);
			return true;
		}); 
		
		instant_message::on_message('f', [=](auto& msg, const auto& sender_name, const auto& sender_id)
		{
			PRINT_MESSAGE("Instant Message", "Popup attempt caught from '%s' (%llu)", sender_name, sender_id);
			return true;
		});

		instant_message::on_message('h', [=](auto& msg, const auto& sender_name, const auto& sender_id)
		{
			const auto length{ msg.cursize - msg.readcount }; 
		
			char buffer[2048] = { 0 };
			game::call(offsets::MSG_ReadData, &msg, buffer, sizeof buffer, length);

			if (msg.overflowed)
				return false;
			
			if (!game::call<bool>(offsets::LobbyMsgRW_PrepReadMsg, &msg, buffer, length))
				return false;

			if (lobby_msg::log_messages)
				PRINT_LOG("Received lobby message <%s> from '%s' (%llu)", game::LobbyTypes_GetMsgTypeName(msg.type), sender_name, sender_id);

			if (msg.type == game::MESSAGE_TYPE_INFO_REQUEST)
			{
				PRINT_MESSAGE("Instant Message", "Received a info request from '%s' (%llu)", sender_name, sender_id);
				return events::prevent_join;
			}
		
			return false;
		});

		scheduler::on_game_initialized([]()
		{
			if (const auto lobby = game::call<uintptr_t>(offsets::dwGetLobby, 0))
			{
				lobby_event_handlers_vmt.setup(*reinterpret_cast<uintptr_t**>(lobby + 0x750));
				lobby_event_handlers_vmt.hook(27, on_global_instant_message);
			}
		}, scheduler::pipeline::main);
	}
}