#include "dependencies/stdafx.hpp"
#include "connectionless_packet.hpp"

namespace events::connectionless_packet
{
	utils::hook::detour cl_dispatch_connectionless_packet_hook;

	bool log_commands = true;

	namespace
	{
		using callback = std::function<bool(const command::args&, const game::netadr_t&, game::msg_t& msg)>;
		std::unordered_map<std::string, callback>& get_callbacks()
		{
			static std::unordered_map<std::string, callback> callbacks{};
			return callbacks;
		}

		void on_command(const std::string& command, const callback& callback)
		{
			get_callbacks()[utils::string::to_lower(command)] = callback;
		}
	}

	bool handle_command(const game::netadr_t& from, game::msg_t* msg, bool server_oob)
	{
		const auto args = command::args{};

		const auto msg_backup = *msg;

		char buffer[1024] = { 0 };
		args.tokenize(msg->read_string(buffer, true));
		const auto _ = utils::finally([=]()
		{
			*msg = msg_backup;
			args.end_tokenize();
		});

		const auto oob_string = args.join();
		const auto ip_str = utils::get_sender_string(from);

		if (log_commands)
		{
			PRINT_LOG("Received OOB [%c] '%s' from %s", server_oob ? 's' : 'c', oob_string.data(), ip_str.data());
		}

		if (const auto write_length = static_cast<uint32_t>(msg->cursize) + 48; 
			sizeof(game::PacketQueueBlock::data) < write_length)
		{
			PRINT_LOG("Ignoring OOB of size [%u] from %s", msg->cursize, ip_str.data());
			return true;
		}

		const auto& callbacks = get_callbacks();
		const auto handler = callbacks.find(utils::string::to_lower(args[0]));

		if (handler == callbacks.end())
			return false;

		return handler->second(args, from, *msg);
	}

	void __fastcall cl_dispatch_connectionless_packet(int localClientNum, game::netadr_t from, game::msg_t* msg)
	{
		if (connectionless_packet::handle_command(from, msg))
			return;

		return cl_dispatch_connectionless_packet_hook.call(localClientNum, from, msg);
	}

	void initialize()
	{
		cl_dispatch_connectionless_packet_hook.create(OFFSET(0x7FF7170D24B0), cl_dispatch_connectionless_packet);
			
		const auto ignore_oob = [](const auto& args, const auto& from, auto&)
		{
			PRINT_LOG("Ignoring OOB '%s' from %s", args.join(0).data(), utils::get_sender_string(from).data());
			return true;
		};
		
		connectionless_packet::on_command("mstart", ignore_oob);
		connectionless_packet::on_command("connectResponseMigration", ignore_oob);
		connectionless_packet::on_command("print", ignore_oob);
		connectionless_packet::on_command("echo", ignore_oob);
		connectionless_packet::on_command("LM", [](const auto&, const auto& from, auto& msg)
		{
			return lobby_msg::handle(from, &msg, game::NETCHAN_INVALID_CHANNEL);
		});
	}
}