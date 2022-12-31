#include "dependencies/std_include.hpp"
#include "string.hpp"

namespace utils::string
{
	std::string get_log_file(const std::string& dir, const std::string& ext)
	{
		return "furtivehook/" + dir + "/furtivehook-" + utils::string::data_time() + "." + ext;
	}

	std::string data_time(time_t seconds, const bool date_only)
	{
		const auto time = seconds == 0 ? std::time(nullptr) : seconds;
		const auto local_time = std::localtime(&time);

		char timestamp[MAX_PATH] = { 0 };

		if (date_only)
		{
			std::strftime(timestamp, sizeof(timestamp), "%F", local_time);
		}
		else
		{
			std::strftime(timestamp, sizeof(timestamp), "%F %r", local_time);
		}

		return timestamp;
	}
	
	std::string join(const std::vector<std::string>& args, const std::size_t index)
	{
		auto result = ""s;

		for (auto i = index; i < args.size(); ++i)
		{
			if (i > index)
			{
				result.append(" ");
			}

			result.append(args[i]);
		}

		return result;
	}
	
	std::string dump_hex(const std::string& data, const std::string& separator)
	{
		auto result = ""s;

		for (size_t i = 0; i < data.size(); ++i)
		{
			if (i > 0)
			{
				result.append(separator);
			}

			result.append(va("%02X", data[i] & 0xFF));
		}

		return result;
	}
	
	std::string format(const va_list ap, const char* message)
	{
		static thread_local char buffer[0x1000];

		const auto count = vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, message, ap);

		if (count < 0) return {};
		return { buffer, static_cast<size_t>(count) };
	}
	
	std::string va(const char* fmt, ...)
	{
		static thread_local va_provider<8, 256> provider;

		va_list ap;
		va_start(ap, fmt);

		const auto* result = provider.get(fmt, ap);

		va_end(ap);
		return result;
	}

	std::vector<std::string> split(const std::string& text, const char delimiter)
	{
		std::stringstream ss(text);
		std::string item;
		std::vector<std::string> elems;

		while (std::getline(ss, item, delimiter))
		{
			if (!item.empty())
			{
				elems.emplace_back(item);
			}
		}

		return elems;
	}

	std::string replace_all(std::string str, const std::string& from, const std::string& to)
	{
		if (from.empty())
		{
			return str;
		}
		
		std::string::size_type start_pos = 0;

		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}

		return str;
	}

	bool begins_with(const std::string& text, const std::string& substring)
	{
		return text.find(substring) == 0;
	}

	bool contains(const std::string &text, const std::string &substring)
	{
		return text.find(substring) != std::string::npos;
	}

	bool ends_with(const std::string& text, const std::string& substring)
	{
		if (substring.size() > text.size()) return false;
		return std::equal(substring.rbegin(), substring.rend(), text.rbegin());
	}

	std::string to_lower(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](const auto& input) { return static_cast<char>(std::tolower(input)); });
		return text;
	}

	std::string to_upper(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](const auto& input) { return static_cast<char>(std::toupper(input)); });
		return text;
	}

	std::string convert(const std::wstring& wstr)
	{
		std::string result;
		result.reserve(wstr.size());

		for (const auto& chr : wstr)
		{
			result.push_back(static_cast<char>(chr));
		}

		return result;
	}

	std::wstring convert(const std::string& str)
	{
		std::wstring result;
		result.reserve(str.size());

		for (const auto& chr : str)
		{
			result.push_back(static_cast<wchar_t>(chr));
		}

		return result;
	}

	std::string reverse_words(std::string_view s)
	{
		std::string result;
		result.reserve(s.size());

		while (!s.empty())
		{
			const auto i = s.rfind(' ');
			result.append(s.begin() + i + 1, s.end());

			if (i == std::string_view::npos) break;

			s = s.substr(0, i);
		}

		return result;
	}

	std::string generate_random_string(const size_t length)
	{
		const auto random_characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"s;
		auto result = ""s;

		std::random_device device;
		std::mt19937 engine(device());
		std::uniform_int_distribution<std::uint32_t> dist(0, random_characters.size() - 1);

		for (size_t i = 0; i < length; ++i)
		{
			const auto random_index = dist(engine);
			result += random_characters[random_index];
		}

		return result;
	}

	std::string strip_colors(const std::string& string)
	{
		return std::regex_replace(string, std::regex{ "\\^[\\d;HIBF]" }, "");
	}

	void strip_materials(char* string)
	{
		for (size_t i = 0; i < std::strlen(string); ++i)
		{
			if (string[i] == '^')
			{
				const auto material_char{ &string[i + 1] };
				
				if (*material_char == 'H' || *material_char == 'I' || *material_char == 'B')
				{
					string[i] = '.';
					++i;
				}
			}
		}
	}
}