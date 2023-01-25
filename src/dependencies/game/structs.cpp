#include "dependencies/stdafx.hpp"
#include "structs.hpp"

namespace game
{
	bool msg_t::init_lobby_read()
	{
		this->packageType = game::PACKAGE_TYPE_READ;
		this->encodeFlags = game::call<uint8_t>(0x7FF794B418B0, this, 2);

		if (this->read<uint8_t>() != 5)
		{
			this->overflowed = true;
			return false;
		}

		this->type = static_cast<game::MsgType>(this->read<uint8_t>());

		if (this->overflowed)
		{
			return false;
		}

		if (this->read<uint8_t>() != 11)
		{
			this->overflowed = true;
			return false;
		}

		char msg_type_name[32] = { 0 };
		this->read_string(msg_type_name);

		size_t i = 0;

		while (!this->overflowed)
		{
			if (const auto element_type = read<uint8_t>(); element_type == MESSAGE_ELEMENT_DEBUG_START
				&& ++i == MESSAGE_TYPE_COUNT)
			{
				PRINT_MESSAGE("LobbyMSG", "Potential stack overflow in '%s'", game::LobbyTypes_GetMsgTypeName(this->type));
				return false;
			}
		}

		return true;
	}
}
