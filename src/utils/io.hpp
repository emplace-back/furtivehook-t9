#pragma once
#include "dependencies/std_include.hpp"

namespace utils::io
{
	bool write_file(const std::string & file, const std::string & data, const bool append = false);
	bool create_directory(const std::string & directory);
	bool remove_file(const std::string & file);
	bool move_file(const std::string & src, const std::string & target);
	bool file_exists(const std::string & file);
	std::string read_file(const std::string & file);
	bool read_file(const std::string & file, std::string * data);
}