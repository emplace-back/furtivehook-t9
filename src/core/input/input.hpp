#pragma once
#include "dependencies/stdafx.hpp"

struct hotkey_t
{
	UINT key;
	void(*func)();
};

namespace input
{
	void on_key(UINT key, void(*cb)());
	void initialize();
	extern HWND hwnd_;
}