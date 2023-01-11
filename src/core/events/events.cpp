#include "dependencies/std_include.hpp"
#include "events.hpp"

namespace events
{
	bool prevent_join = true;
	
	void initialize()
	{
		lobby_msg::initialize(); 
		instant_message::initialize();
		connectionless_packet::initialize();
	}
}