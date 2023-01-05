#include "dependencies/std_include.hpp"
#include "misc.hpp"

namespace misc
{
	void initialize()
	{
		scheduler::once(game::initialize, scheduler::pipeline::main);

		input::on_key(VK_F2, [] { game::Cbuf_AddText("disconnect"); });
		input::on_key(VK_F3, [] { game::Cbuf_AddText("quit"); });
	}
}