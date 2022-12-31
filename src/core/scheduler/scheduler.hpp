#pragma once
#include "dependencies/std_include.hpp"

namespace scheduler
{
	struct task
	{
		std::function<bool()> handler{};
		std::chrono::milliseconds interval{};
		std::chrono::high_resolution_clock::time_point last_call{};
	};

	enum pipeline
	{
		main,
		renderer,
		backend,
		count,
	};

	using task_list = std::vector<task>;
	static const bool cond_continue = false;
	static const bool cond_end = true;

	void execute(const pipeline type = pipeline::main);
	void schedule(const std::function<bool()>& callback, const pipeline type = pipeline::main, const std::chrono::milliseconds delay = 0ms);
	void loop(const std::function<void()>& callback, const pipeline type = pipeline::main, const std::chrono::milliseconds delay = 0ms);
	void once(const std::function<void()>& callback, const pipeline type = pipeline::main, const std::chrono::milliseconds delay = 0ms);
	void on_game_initialized(const std::function<void()>& callback, const pipeline type = pipeline::main, const std::chrono::milliseconds delay = 0ms);
	void initialize();
}