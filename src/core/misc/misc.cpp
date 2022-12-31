#include "dependencies/std_include.hpp"
#include "misc.hpp"

namespace misc
{
	void initialize()
	{
		scheduler::once(game::initialize, scheduler::pipeline::main);
	}
}