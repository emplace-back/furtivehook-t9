#pragma once
#include "dependencies/stdafx.hpp"

namespace command
{
	void execute(std::string command, const bool sync = false);

	class args final
	{
	public:
		args() : cmd_args(game::Sys_GetTLS()->cmdArgs) {}

		void tokenize(const char* string) const;
		void end_tokenize() const;
		const char* get(const int index) const;
		int size() const;
		std::string join(const int index = 0) const;

		const char* operator[](const int index) const
		{
			return this->get(index);
		}
	private:
		game::CmdArgs* cmd_args{};
	};
}
