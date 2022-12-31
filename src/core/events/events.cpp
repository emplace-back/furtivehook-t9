#include "dependencies/std_include.hpp"
#include "events.hpp"

namespace events
{
	bool prevent_join = false;
	
	void initialize()
	{
		instant_message::initialize();
		lobby_msg::initialize();
	}
}