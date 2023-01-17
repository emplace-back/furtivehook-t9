#include "dependencies/stdafx.hpp"
#include "command.hpp"

namespace command
{
	namespace
	{
		using callback = std::function<bool(const command::args&)>; 
		
		std::unordered_map<std::string, callback>& get_callbacks()
		{
			static std::unordered_map<std::string, callback> callbacks{};
			return callbacks;
		}
	}

	void args::tokenize(const char* text)
	{
		if (auto& nesting{ ++cmd_args->nesting }; nesting < 8)
		{
			cmd_args->usedTextPool[nesting] = -cmd_args->totalUsedTextPool;
			cmd_args->localClientNum[nesting] = -1;
			cmd_args->controllerIndex[nesting] = 0;
			cmd_args->argv[nesting] = &cmd_args->argvPool[cmd_args->totalUsedArgvPool];
			cmd_args->argshift[nesting] = 0;
			cmd_args->argc[nesting] = game::call<int>(offsets::Cmd_TokenizeStringInternal, text, 512 - cmd_args->totalUsedArgvPool, false, cmd_args->argv[nesting], cmd_args);
			cmd_args->totalUsedArgvPool += cmd_args->argc[nesting];
			cmd_args->usedTextPool[nesting] += cmd_args->totalUsedTextPool;
		}
	}

	void args::end_tokenize()
	{
		if (auto& nesting{ cmd_args->nesting }; nesting >= 0 && nesting < 8)
		{
			cmd_args->totalUsedArgvPool -= cmd_args->argc[nesting];
			cmd_args->totalUsedArgvPool -= cmd_args->argshift[nesting];
			cmd_args->totalUsedTextPool -= cmd_args->usedTextPool[nesting];
			--nesting;
		}
	}
	
	const char* args::get(const int index) const
	{
		if (index < 0 || index >= this->size())
		{
			return "";
		}

		return cmd_args->argv[cmd_args->nesting][index];
	}

	int args::size() const
	{
		return cmd_args->argc[cmd_args->nesting];
	}

	std::string args::join(const int index) const
	{
		if (index < 0)
		{
			return "";
		}
		
		std::string result{};

		for (auto i = index; i < this->size(); ++i)
		{
			if (i > index) result.append(" ");
			result.append(this->get(i));
		}

		return result;
	}

	void execute(std::string command, const bool sync)
	{
		command += "\n";

		if (sync)
		{

		}
		else
		{
			game::Cbuf_AddText(command.data());
		}
	}
}