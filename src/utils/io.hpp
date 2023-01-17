#pragma once
#include "dependencies/stdafx.hpp"

namespace utils::io
{
	bool write_file(const std::string & file, const std::string & data, const bool append = false);
	bool create_directory(const std::string & directory);
	bool remove_file(const std::filesystem::path& file);
	bool move_file(const std::filesystem::path& src, const std::filesystem::path& target);
	bool file_exists(const std::string & file);
	std::string read_file(const std::string & file);
	bool read_file(const std::string & file, std::string * data);
}