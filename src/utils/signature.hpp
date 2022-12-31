#pragma once
#include "dependencies/std_include.hpp"
#include "nt.hpp"
#include <cstdint>

namespace utils::hook
{
	uint8_t* scan_pattern(const char* signature);
	uint8_t* scan_pattern(const char* mod, const char* signature);
	uint8_t* scan_pattern(const utils::nt::library& mod, const char* signature);
}

uintptr_t operator"" _sig(const char* str, size_t);
