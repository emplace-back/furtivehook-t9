#pragma once
#include "dependencies/stdafx.hpp"

namespace exception::dvars
{
	enum hook_dvar
	{
		handle_packet = 1,
		netchan_debugspew = 2,
		unlockables = 3,
	};
	
	bool handle_exception(PEXCEPTION_POINTERS ex);
	void initialize();
}