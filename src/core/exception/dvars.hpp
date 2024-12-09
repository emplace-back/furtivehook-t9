#pragma once
#include "dependencies/stdafx.hpp"

namespace exception::dvars
{
	enum hook_dvar
	{
		handle_packet = 1,
		unlockables = 2,
	};
	
	bool handle_exception(PEXCEPTION_POINTERS ex);
	void initialize();
}