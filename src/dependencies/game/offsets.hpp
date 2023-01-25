#pragma once

namespace offsets
{
	constexpr auto dump_base = 0x7FF78EC30000;
	constexpr auto jmp_qword_ptr_rbx = 0x7FF7A7FD5CDB;
	
	constexpr auto Cmd_TokenizeStringInternal = 0x7FF79868A8E0;
	constexpr auto NET_OutOfBandData = 0x7FF7987BADD0;
	constexpr auto Netchan_SendMessage = 0x7FF7987BE040;
	constexpr auto LobbyJoin_ConnectToHost = 0x7FF799F69540;
	constexpr auto LobbyNetChan_GetLobbyChannel = 0x7FF79A06DE60;
	constexpr auto Live_IsUserSignedInToDemonware = 0x7FF7982A5560;
	constexpr auto LiveUser_GetXuid = 0x7FF79B62A5F0;
	constexpr auto InfoResponse = 0x7FF798BE3580;
	constexpr auto LobbyMsgRW_PrepReadMsg = 0x7FF799F45550;
	constexpr auto dwGetLobby = 0x7FF799913060;
	constexpr auto dwNetadrToCommonAddr = 0x7FF799A5A2A0;
	constexpr auto dwInstantSendMessage = 0x7FF7998E3300;
	constexpr auto Sys_GetTLS = 0x7FF79AC835F0;
	constexpr auto ret_com_switch_mode = 0x7FF6E6839AAB;

	constexpr auto s_lobbyMsgName = 0x7FF79CD6CAC0;
	constexpr auto s_clientSession = 0x7FF7A1FC47D0;
	constexpr auto playerInventory = 0x7FF7A5AF7070;
}