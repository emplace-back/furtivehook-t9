#pragma once

namespace offsets
{
	constexpr auto dump_base = 0x7FF6E1080000;
	constexpr auto jmp_qword_ptr_rbx = 0x7FF6FC97A72B;
	
	constexpr auto Cmd_TokenizeStringInternal = 0x7FF6EAC30670;
	constexpr auto NET_OutOfBandData = 0x7FF6EAD5B5F0;
	constexpr auto Netchan_SendMessage = 0x7FF6EAD5E860;
	constexpr auto LobbyJoin_ConnectToHost = 0x7FF6EC57EAC0;
	constexpr auto LobbyNetChan_GetLobbyChannel = 0x7FF6EC68B1F0;
	constexpr auto Live_IsUserSignedInToDemonware = 0x7FF6EA837B30;
	constexpr auto Live_GetXuid = 0x7FF6EDC6A440;
	constexpr auto InfoResponse = 0x7FF6EB190E80;
	constexpr auto LobbyMsgRW_PrepReadMsg = 0x7FF6EC559B20;
	constexpr auto dwGetLobby = 0x7FF6EBF0C5B0;
	constexpr auto dwNetadrToCommonAddr = 0x7FF6EC05A320;
	constexpr auto dwInstantSendMessage = 0x7FF6EBEDBB90;
	constexpr auto Sys_GetTLS = 0x7FF6ED2BC490;
	constexpr auto MSG_ReadStringLine = 0x7FF6E7036C20;
	constexpr auto ret_com_switch_mode = 0x7FF6E6839AAB;

	constexpr auto s_lobbyMsgName = 0x7FF6EF3B77C0;
	constexpr auto s_clientSession = 0x7FF6F45F2D50;
}