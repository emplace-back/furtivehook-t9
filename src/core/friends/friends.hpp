#pragma once
#include "dependencies/std_include.hpp"

namespace friends
{	
	struct friend_info
	{
		std::uint64_t id;
		std::string name;
		std::time_t last_online;
		game::Msg_InfoResponse response;

		bool is_online() const noexcept
		{
			return response.nonce;
		}

		bool operator<(const friend_info& other) const
		{
			if (is_online())
			{
				return is_online() > other.is_online();
			}
			
			return last_online > other.last_online;
		}
	}; 

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(friend_info, id, name, last_online)

	friend_info* get(const uint64_t id);
	void write();
	void draw_friends_list(const float width, const float spacing);
	void initialize();

	extern bool fetch;
	extern std::vector<friend_info> friends;
	extern uint32_t NONCE;
}