#include "dependencies/stdafx.hpp"
#include "structs.hpp"

namespace game 
{
	netadr_t InfoResponseLobby::get_netadr() const
	{
		return game::net::oob::register_remote_addr(*this);
	}
	
	bool msg_t::init_lobby_read()
	{
		this->packageType = game::PACKAGE_TYPE_READ;
		this->encodeFlags = read<uint8_t>();

		if (this->read<uint8_t>() != MESSAGE_ELEMENT_UINT8)
		{
			this->overflowed = true;
			return false;
		}

		this->type = static_cast<game::MsgType>(this->read<uint8_t>());

		if (this->overflowed)
		{
			return false;
		}

		if (this->read<uint8_t>() != MESSAGE_ELEMENT_STRING)
		{
			this->overflowed = true;
			return false;
		}

		char msg_type_name[32] = { 0 };
		this->read_string(msg_type_name);

		auto debug_message_overflow = ""s;
		debug_message_overflow.resize(MESSAGE_TYPE_COUNT, MESSAGE_ELEMENT_DEBUG_START);

		if (this->get_data().find(debug_message_overflow.data(), 0, debug_message_overflow.size()) != std::string::npos)
		{
			PRINT_MESSAGE("LobbyMSG", "Potential stack overflow in '%s'", game::LobbyTypes_GetMsgTypeName(this->type));
			return false;
		}

		return true;
	}

	size_t msg_t::read_bits_compress(const void* from, size_t fromsize, void* to, size_t tosize) const
	{
		return game::call<size_t>(0x7FF64A6C2190, from, fromsize, to, tosize);
	}
}
