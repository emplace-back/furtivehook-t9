#pragma once
#include "dependencies/std_include.hpp"

namespace command
{
	void execute(std::string command, const bool sync = false);

	class args final
	{
	public:
		args(game::CmdArgs* a) : cmd_args(a)
		{
		}

		static args get_client()
		{
			return args{ game::Sys_GetTLS()->cmdArgs };
		}

		static args get_server()
		{
			return args{ nullptr };
		}

		void tokenize(const char* string);
		void end_tokenize();
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
