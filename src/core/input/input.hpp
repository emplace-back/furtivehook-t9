#pragma once
#include "dependencies/std_include.hpp"

struct hotkey_t
{
	UINT key;
	void(*func)();
	bool block;
};

namespace input
{
	void on_key(UINT key, void(*cb)(), bool block = false); 
	void initialize(HWND hwnd);
}