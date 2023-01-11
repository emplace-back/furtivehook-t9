#include "dependencies/std_include.hpp"
#include "connectionless_packet.hpp"

namespace events::connectionless_packet
{
	bool log_commands = true;

	namespace
	{
		using callback = std::function<bool(const command::args&, const game::netadr_t&, game::msg_t&)>;
		std::unordered_map<std::string, callback>& get_callbacks()
		{
			static std::unordered_map<std::string, callback> callbacks{};
			return callbacks;
		}

		void on_command(const std::string& command, const callback& callback)
		{
			get_callbacks()[utils::string::to_lower(command)] = callback;
		}

		bool handle_command(const char* buffer, const game::netadr_t& from, game::msg_t* msg)
		{
			auto args = command::args::get_client();
			args.tokenize(buffer);
			const auto _ = utils::finally([&args]()
			{
				args.end_tokenize();
			});

			if (log_commands)
			{
				PRINT_LOG("Received OOB '%s' from %s", args.join(0).data(), utils::get_sender_string(from).data());
			}

			const auto oob_string = utils::string::to_lower(args[0]);
			const auto& callbacks = get_callbacks();
			const auto handler = callbacks.find(oob_string);

			if (handler == callbacks.end())
				return false;

			return handler->second(args, from, *msg);
		}
	}

	bool handle_command(const game::netadr_t& from, game::msg_t* msg, const bool header)
	{
		if(header) msg->read<int>();
		
		char buffer[0x400] = { 0 };
		game::call<char*>(0x7FF7D5F9A920, msg, buffer, sizeof buffer);

		return handle_command(buffer, from, msg);
	}

	void initialize()
	{
		const auto ignore_oob = [](const command::args& args, const game::netadr_t& from, auto&)
		{
			PRINT_LOG("Ignoring OOB '%s' from %s", args.join(0).data(), utils::get_sender_string(from).data());
			return true;
		};
		
		connectionless_packet::on_command("mstart", ignore_oob);
		connectionless_packet::on_command("mhead", ignore_oob);
		connectionless_packet::on_command("mstate", ignore_oob);
		connectionless_packet::on_command("connectResponseMigration", ignore_oob);
		connectionless_packet::on_command("print", ignore_oob);
		connectionless_packet::on_command("echo", ignore_oob);
		connectionless_packet::on_command("rcon", ignore_oob);
		connectionless_packet::on_command("RA", ignore_oob);
	}
}