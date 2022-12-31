#include "dependencies/std_include.hpp"
#include "io.hpp"

namespace utils::io
{
	bool write_file(const std::string& file, const std::string& data, const bool append)
	{
		if (const auto pos = file.find_last_of("/\\"); pos != std::string::npos)
		{
			create_directory(file.substr(0, pos));
		}

		std::ofstream stream(file, std::ios::binary | std::ofstream::out | (append ? std::ofstream::app : 0));

		if (stream.is_open())
		{
			stream.write(data.data(), data.size());
			stream.close();

			return true;
		}

		return false;
	}
	
	bool create_directory(const std::string& directory)
	{
		return std::filesystem::create_directories(directory);
	}

	bool remove_file(const std::string& file)
	{
		return DeleteFileA(file.data()) == TRUE;
	}

	bool move_file(const std::string& src, const std::string& target)
	{
		return MoveFileA(src.data(), target.data()) == TRUE;
	}
	
	bool file_exists(const std::string& file)
	{
		return std::ifstream(file).good();
	}

	std::string read_file(const std::string& file)
	{
		std::string data;
		read_file(file, &data);
		return data;
	}

	bool read_file(const std::string& file, std::string* data)
	{
		if (!data) return false;
		data->clear();

		if (file_exists(file))
		{
			std::ifstream stream(file, std::ios::binary);
			if (!stream.is_open()) return false;

			stream.seekg(0, std::ios::end);
			const std::streamsize size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			if (size > -1)
			{
				data->resize(static_cast<uint32_t>(size));
				stream.read(const_cast<char*>(data->data()), size);
				stream.close();
				return true;
			}
		}

		return false;
	}
}