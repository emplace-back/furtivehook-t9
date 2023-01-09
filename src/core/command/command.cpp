#include "dependencies/std_include.hpp"
#include "command.hpp"

namespace command
{
	void execute(std::string command, const bool sync)
	{
		command += "\n";

		if (sync)
		{
			
		}
		else
		{
			game::Cbuf_AddText(command.data());
		}
	}
}