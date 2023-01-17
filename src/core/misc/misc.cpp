#include "dependencies/stdafx.hpp"
#include "misc.hpp"

namespace misc
{
	void remove_crash_file()
	{
		const utils::nt::library game{};
		const auto game_file = game.get_path();
		auto game_path = std::filesystem::path(game_file);
		game_path.replace_extension(".start");

		utils::io::remove_file(game_path);
	}
	
	void initialize()
	{
		misc::remove_crash_file();
		
		scheduler::once(game::initialize, scheduler::pipeline::main);

		input::on_key(VK_F2, [] { command::execute("disconnect"); });
		input::on_key(VK_F3, [] { command::execute("quit"); });
	}
}