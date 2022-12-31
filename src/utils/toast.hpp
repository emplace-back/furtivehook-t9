#pragma once
#include "dependencies/std_include.hpp"

namespace utils::toast
{
	struct toast_data
	{
		std::string title;
		std::string message;
		std::chrono::milliseconds interval{};
		std::chrono::high_resolution_clock::time_point last_time{};
	};

	void add_toast(const std::string & title, const std::string & message, const std::chrono::milliseconds & interval = 5000ms);
	void add_toast(const toast_data& toast);
	void draw_toast();
}