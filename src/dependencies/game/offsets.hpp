#pragma once

namespace offsets
{
	constexpr auto dump_base = 0x7FF7D0110000;
	constexpr auto jmp_qword_ptr_rbx = 0x7FF7EB3AC102;
	
	constexpr auto IN_Activate = 0x7FF7DCCCB2C0;
	constexpr auto NET_OutOfBandData = 0x7FF7D9BD28D0;
	constexpr auto Live_IsUserSignedInToDemonware = 0x7FF7D96BF190;
	constexpr auto Live_GetXuid = 0x7FF7DC9F1E80;
	constexpr auto InfoResponse = 0x7FF7DA003F70;
	constexpr auto LobbyMsgRW_PrepReadMsg = 0x7FF7DB326B50;
	constexpr auto dwGetLobby = 0x7FF7DAD00190;
	constexpr auto dwNetadrToCommonAddr = 0x7FF7DAE42DF0;
	constexpr auto dwInstantSendMessage = 0x7FF7DACD0F80;
	
	constexpr auto s_lobbyMsgName = 0x7FF7DE121840;
	constexpr auto s_clientSession = 0x7FF7E3362AD0;
	constexpr auto cmd_text_buf = 0x7FF7E1FBB250;
}