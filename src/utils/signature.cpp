#include "dependencies/std_include.hpp"
#include "signature.hpp"

namespace utils::hook
{
	uint8_t* scan_pattern(const char* signature)
	{
		return utils::hook::scan_pattern(utils::nt::library{}, signature);
	}

	uint8_t* scan_pattern(const char* mod, const char* signature)
	{
		return utils::hook::scan_pattern(utils::nt::library{ mod }, signature);
	}

	uint8_t* scan_pattern(const utils::nt::library& mod, const char* signature)
	{
		static auto pattern_to_byte = [](const char* signature)
		{
			std::vector<uint8_t> bytes{};

			for (auto start = signature; start < signature + std::strlen(signature); ++start)
			{
				if (*start == '?')
				{
					++start;
					if (*start == '?') ++start;

					bytes.push_back(0xff);
				}
				else
				{
					bytes.push_back(static_cast<uint8_t>(std::strtoul(start, const_cast<char**>(&start), 16)));
				}
			}

			return bytes;
		};

		if (!mod)
		{
			DEBUG_LOG("Invalid module for signature: %s", signature);
			return nullptr;
		}
		
		const auto image_size = mod.get_optional_header()->SizeOfImage;
		const auto pattern = pattern_to_byte(signature); 
		const auto pattern_size = pattern.size();
		const auto pattern_bytes = pattern.data();
		const auto bytes = mod.get_ptr();

		MEMORY_BASIC_INFORMATION mbi{};
		for (auto page = bytes;
			page < reinterpret_cast<uint8_t*>(bytes + image_size);
			page = reinterpret_cast<uint8_t*>(uintptr_t(mbi.BaseAddress) + mbi.RegionSize))
		{
			VirtualQuery(page, &mbi, sizeof(mbi));

			for (auto i = uintptr_t(mbi.BaseAddress) - uintptr_t(bytes); i < mbi.RegionSize; ++i)
			{
				if (i + pattern_size == mbi.RegionSize)
				{
					VirtualQuery(page, &mbi, sizeof(mbi));

					if (mbi.Protect != PAGE_EXECUTE_READ)
						break;
				}

				auto found = true;
				
				for (size_t j = 0; j < pattern_size; ++j)
				{
					if (pattern_bytes[j] != 0xff && pattern_bytes[j] != bytes[i + j])
					{
						found = false;
						break;
					}
				}

				if (found)
				{
					return &bytes[i];
				}
			}
		}

		DEBUG_LOG("Failed to find signature: %s", signature);
		return nullptr;
	}
}

uintptr_t operator"" _sig(const char* str, const size_t)
{
	return reinterpret_cast<uintptr_t>(utils::hook::scan_pattern(str));
}
