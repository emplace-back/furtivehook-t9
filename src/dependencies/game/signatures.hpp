#pragma once

namespace signatures
{
	constexpr auto littlelong_ptr = "E8 ?? ?? ?? ?? 89 44 24 20 89 73 10";
	constexpr auto unlockables_ptr = "48 8B 0D ?? ?? ?? ?? 8B F8 45 32 F6";
	constexpr auto swapchain_ptr = "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 0D 48 8B 11";
	constexpr auto lobbymsg_prints_ptr = "48 8B 0D ?? ?? ?? ?? 49 8B E8 48 8B F2";
	constexpr auto msg_read_long = "48 89 5C 24 10 57 48 83 EC 20 48 63 41 10 48 8B D9 8D 78 04";
	constexpr auto ret_com_client_packet_event = "0F 28 44 24 40 4C 8D 44 24 50 44 8B C8 66 0F 7F 44 24 30 48 8D 54 24 30 8B CB";
	constexpr auto netchan_debugspew_ptr = "48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 84 C0 75 0C 48 8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 45 84 E4";
	constexpr auto dispatch_hook_a_ptr = "40 55 53 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 78 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 45 F0 65";
}