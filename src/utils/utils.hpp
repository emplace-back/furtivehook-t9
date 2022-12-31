#pragma once
#include "dependencies/std_include.hpp"

namespace utils
{
	bool are_addresses_equal(const game::netadr_t& a, const game::netadr_t& b);
	std::string get_sender_string(const game::netadr_t& from);
	void print_log(const char* msg, ...);

	template <typename T> auto atoi(const std::string& str)
	{
		return static_cast<T>(std::atoi(str.data()));
	}

	static auto atoi(const std::string& str)
	{
		return atoi<int>(str);
	}
}

inline bool operator==(const game::netadr_t& a, const game::netadr_t& b)
{
	return utils::are_addresses_equal(a, b);
}

inline bool operator!=(const game::netadr_t& a, const game::netadr_t& b)
{
	return !(a == b); 
}

namespace std
{
	template <>
	struct equal_to<game::netadr_t>
	{
		using result_type = bool;

		bool operator()(const game::netadr_t& lhs, const game::netadr_t& rhs) const
		{
			return utils::are_addresses_equal(lhs, rhs);
		}
	}; 
	
	template <>
	struct hash<game::netadr_t>
	{
		size_t operator()(const game::netadr_t& x) const noexcept
		{
			const auto type_hash = hash<uint32_t>()(x.type);

			if (x.type != game::NA_IP && x.type != game::NA_RAWIP)
			{
				return type_hash;
			}

			return type_hash ^ hash<uint32_t>()(x.inaddr) ^ hash<uint16_t>()(x.port);
		}
	};

	template<typename T1, typename T2>
	struct hash<pair<T1, T2>>
	{
		size_t operator()(const pair<T1, T2>& x) const
		{
			return hash<T1>()(x.first) ^ hash<T2>()(x.second);
		}
	};
}