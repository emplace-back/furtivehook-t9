#pragma once

namespace offsets
{
	constexpr auto dump_base = 0x7FF6F3A20000;
	constexpr auto jmp_qword_ptr_rbx = 0x7FF7126B12AF;
	constexpr auto swapchain = 0x7FF70AFEDD58;
	
	constexpr auto BigShort = 0x7FF70DB83F00;
	constexpr auto BigLong = 0x7FF70DB83F10;
	constexpr auto MSG_ReadData = 0x7FF6F99341C0;
	constexpr auto CL_AddReliableCommand = 0x7FF6F96BAFA0;
	constexpr auto Live_IsUserSignedInToDemonware = 0x7FF6FD1CA300;
	constexpr auto Live_GetXuid = 0x7FF7006358D0;
	constexpr auto LobbyMsgRW_PrepReadMsg = 0x7FF6FEEEDD20;
	constexpr auto LobbyMsgRW_PrepReadMsg_ = 0x7FF6FEEEE340;
	constexpr auto dwGetLobby = 0x7FF6FE8A34A0;
	constexpr auto dwNetadrToCommonAddr = 0x7FF6FE9F1F30;
	constexpr auto dwInstantSendMessage = 0x7FF6FE873070;
	constexpr auto dwInstantHandleRemoteCommand = 0x7FF6FE872850;
	
	constexpr auto s_lobbyMsgName = 0x7FF701D63180;
	constexpr auto s_clientSession = 0x7FF706FBA550;
	
	constexpr auto ret_sys_ms = 0x7FF70076A686;
	constexpr auto ret_com_frame = 0x7FF6FB0C1B55;
	constexpr auto ret_msg_read_long = 0x7FF6F994334F;
	constexpr auto ret_msg_read_short = 0x7FF6F9943B5C;
	constexpr auto ret_lobby_msg_rw_package_int = 0x7FF6FEEED87C;
	constexpr auto ret_handle_packet_1 = 0x7FF6FEA758A8;
	constexpr auto ret_handle_packet_2 = 0x7FF6FEA75B4F;
	constexpr auto ret_handle_packet_3 = 0x7FF6FEA75CEF;
}