#pragma once
#include "dependencies/std_include.hpp"

namespace menu
{
	constexpr auto window_title = "furtivehook";
	constexpr auto window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	constexpr auto hotkey = VK_F1;
	extern ImFont* glacial_indifference_bold; extern ImFont* glacial_indifference;
	extern bool open;

	void set_style();
	void toggle();
	void draw();
}