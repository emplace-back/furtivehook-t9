#pragma once
#include "dependencies/stdafx.hpp"

namespace command
{
	void execute(std::string command, const bool sync = false);

	class args final
	{
	public:
		args() : cmd_args(game::Sys_GetTLS()->cmdArgs) {}

		[[nodiscard]] void tokenize(const char* string) const;
		[[nodiscard]] void end_tokenize() const;
		[[nodiscard]] const char* get(const int index) const;
		[[nodiscard]] int size() const;
		[[nodiscard]] std::string join(const int index = 0) const;

		[[nodiscard]] const char* operator[](const int index) const
		{
			return this->get(index);
		}
	private:
		game::CmdArgs* cmd_args{};
	};
}
