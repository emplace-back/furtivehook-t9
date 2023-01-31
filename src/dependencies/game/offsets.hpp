#pragma once

namespace offsets
{
	constexpr auto dump_base = 0x7FF6440E0000;
	constexpr auto jmp_qword_ptr_rbx = 0x7FF65F6FFFCF;
	
	constexpr auto Cmd_TokenizeStringInternal = 0x7FF64E245A30;
	constexpr auto NET_OutOfBandData = 0x7FF64E378830;
	constexpr auto Netchan_SendMessage = 0x7FF64E37BAA0;
	constexpr auto LobbyJoin_ConnectToHost = 0x7FF64FB402B0;
	constexpr auto LobbyNetChan_GetLobbyChannel = 0x7FF64FC43600;
	constexpr auto Live_IsUserSignedInToDemonware = 0x7FF64DE596B0;
	constexpr auto LiveUser_GetXuid = 0x7FF651208410;
	constexpr auto InfoResponse = 0x7FF64E7A85B0;
	constexpr auto dwGetLobby = 0x7FF64F4DFCB0;
	constexpr auto dwNetadrToCommonAddr = 0x7FF64F625040;
	constexpr auto dwInstantSendMessage = 0x7FF64F4AFC90;
	constexpr auto ret_com_switch_mode = 0x7FF649EE71BA;

	constexpr auto s_lobbyMsgName = 0x7FF6529A9380;
	constexpr auto s_clientSession = 0x7FF657BEDCD0;
	constexpr auto playerInventory = 0x7FF65B720570;
}