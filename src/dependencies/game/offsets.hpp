#pragma once

namespace offsets
{
	constexpr auto dump_base = 0x7FF7113E0000;
	constexpr auto jmp_qword_ptr_rbx = 0x7FF7119A1A44;
	
	constexpr auto Cmd_TokenizeStringInternal = 0x7FF71AF88DD0;
	constexpr auto NET_OutOfBandData = 0x7FF71B0B8630;
	constexpr auto Netchan_SendMessage = 0x7FF71B0BB8A0;
	constexpr auto LobbyJoin_ConnectToHost = 0x7FF71C8AF520;
	constexpr auto LobbyNetChan_GetLobbyChannel = 0x7FF71C9B8A70;
	constexpr auto Live_IsUserSignedInToDemonware = 0x7FF71AB98D50;
	constexpr auto Live_IsDemonwareFetchingDone = 0x7FF71AB98C00;
	constexpr auto LiveUser_GetXuid = 0x7FF71DFBFAD0;
	constexpr auto InfoResponse = 0x7FF71B4E68E0;
	constexpr auto dwGetLobby = 0x7FF71C2498B0;
	constexpr auto dwNetadrToCommonAddr = 0x7FF71C3947B0;
	constexpr auto dwInstantSendMessage = 0x7FF71C2191C0;
	constexpr auto ret_com_switch_mode = 0;

	constexpr auto s_lobbyMsgName = 0x7FF71F27ECD0;
	constexpr auto s_clientSession = 0x7FF7244ECDD0;
	constexpr auto playerInventory = 0x7FF65B720570;
}