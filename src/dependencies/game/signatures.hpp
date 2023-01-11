#pragma once

namespace signatures
{
	constexpr auto biglong_ptr = "E8 ?? ?? ?? ?? 89 44 24 20 89 73 10";
	constexpr auto swapchain_ptr = "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 0D 48 8B 11";
	constexpr auto lobbymsg_prints_ptr = "48 8B 0D ?? ?? ?? ?? 49 8B E8 48 8B F2";
	constexpr auto ret_com_client_packet_event = "0F 28 44 24 40 4C 8D 44 24 50 44 8B C8 66 0F 7F 44 24 30 48 8D 54 24 30 8B CB";
	constexpr auto ret_lobby_msg_rw_package_int = "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 8B 41 44 49 8B F0 48 8B FA 48 8B D9 85 C0 75 10 48 8B 5C 24 30 48 8B 74 24 38 48 83 C4 20 5F C3 83 F8 01 75 28 49 63 30 33";
}