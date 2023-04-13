#include "dependencies/stdafx.hpp"
#include "menu.hpp"

namespace menu
{
	ImFont* glacial_indifference_bold; ImFont* glacial_indifference;
	bool open = true;

	void set_style_color()
	{
		auto& colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_Text] = { 1.0f, 1.0f, 1.0f, 1.0f };
		colors[ImGuiCol_TextDisabled] = { 0.784f, 0.784f, 0.784f, 0.784f };
		colors[ImGuiCol_WindowBg] = { 0.06f, 0.06f, 0.06f, 1.00f };
		colors[ImGuiCol_PopupBg] = { 0.08f, 0.08f, 0.08f, 0.94f };
		colors[ImGuiCol_Border] = { 0.00f, 0.00f, 0.00f, 0.71f };
		colors[ImGuiCol_BorderShadow] = { 0.06f, 0.06f, 0.06f, 0.01f };
		colors[ImGuiCol_FrameBg] = { 0.10f, 0.10f, 0.10f, 0.71f };
		colors[ImGuiCol_FrameBgHovered] = { 0.19f, 0.19f, 0.19f, 0.40f };
		colors[ImGuiCol_FrameBgActive] = { 0.20f, 0.20f, 0.20f, 0.67f };
		colors[ImGuiCol_TitleBg] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_MenuBarBg] = { 0.10f, 0.10f, 0.10f, 0.66f };
		colors[ImGuiCol_ScrollbarBg] = { 0.02f, 0.02f, 0.02f, 0.00f };
		colors[ImGuiCol_ScrollbarGrab] = { 0.31f, 0.31f, 0.31f, 1.00f };
		colors[ImGuiCol_ScrollbarGrabHovered] = { 0.17f, 0.17f, 0.17f, 1.00f };
		colors[ImGuiCol_ScrollbarGrabActive] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_CheckMark] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_SliderGrab] = { 0.29f, 0.29f, 0.29f, 1.00f };
		colors[ImGuiCol_SliderGrabActive] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_Button] = { 0.10f, 0.10f, 0.10f, 1.00f };
		colors[ImGuiCol_ButtonHovered] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_ButtonActive] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_Header] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_HeaderActive] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_Separator] = { 0.10f, 0.10f, 0.10f, 0.90f };
		colors[ImGuiCol_SeparatorHovered] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_SeparatorActive] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_ResizeGrip] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_ResizeGripHovered] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_ResizeGripActive] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_PlotLines] = { 0.61f, 0.61f, 0.61f, 1.00f };
		colors[ImGuiCol_PlotLinesHovered] = { 1.00f, 1.00f, 1.00f, 1.00f };
		colors[ImGuiCol_PlotHistogram] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_PlotHistogramHovered] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_TextSelectedBg] = { 0.0f, 0.392f, 0.784f, 1.0f };
		colors[ImGuiCol_ModalWindowDarkening] = { 0.80f, 0.80f, 0.80f, 0.35f };
	}

	void set_style()
	{
		auto& style = ImGui::GetStyle();
		ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->AddFontDefault();
		glacial_indifference_bold = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(ImGui::GetIO().Fonts->GetSecondaryFont().data(), 15.0f);
		glacial_indifference = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(ImGui::GetIO().Fonts->GetPrimaryFont().data(), 15.0f);

		set_style_color();

		style.WindowPadding = { 8.0f, 2.0f };
		style.FramePadding = { 4.0f , 4.0f };
		style.ItemSpacing = { 10.0f, 4.0f };
		style.ItemInnerSpacing = { 4.0f, 4.0f };
		style.TouchExtraPadding = {};
		style.IndentSpacing = 21.0f;
		style.ScrollbarSize = 13.0f;
		style.GrabMinSize = 10.0f;
		style.WindowBorderSize = 0.0f;
		style.ChildBorderSize = 0.0f;
		style.PopupBorderSize = 0.0f;
		style.FrameBorderSize = 0.0f;
		style.TabBorderSize = 1.0f;

		style.WindowRounding = 7.0f;
		style.ChildRounding = 7.0f;
		style.FrameRounding = 7.0f;
		style.PopupRounding = 7.0f;
		style.ScrollbarRounding = 7.0f;
		style.GrabRounding = 7.0f;
		style.TabRounding = 7.0f;

		style.WindowTitleAlign = { 0.5f, 0.5f };
		style.WindowMenuButtonPosition = ImGuiDir_None;
		style.ButtonTextAlign = { 0.5f, 0.5f };
		style.SelectableTextAlign = {};

		style.DisplaySafeAreaPadding = { 4.0f, 4.0f };

		style.ColumnsMinSpacing = 6.0f;
		style.CurveTessellationTol = 1.25f;
		style.AntiAliasedLines = true;
	}

	void toggle()
	{
		open = !open;
	}

	bool begin_section(const std::string& text)
	{
		ImGui::TextUnformatted(text.data());
		return true;
	}
	
	void draw_player_list(const float width, const float spacing)
	{
		if (ImGui::BeginTabItem("Player List"))
		{
			const auto session = game::session_data();

			if (session == nullptr)
			{
				ImGui::EndTabItem();
				return;
			}

			ImGui::BeginColumns("Players", 2, ImGuiColumnsFlags_NoResize);

			ImGui::SetColumnWidth(-1, 28.0f);
			ImGui::TextUnformatted("#");
			ImGui::NextColumn();
			ImGui::TextUnformatted("Player");
			ImGui::NextColumn();

			ImGui::Separator();

			std::array<std::uint32_t, 18> indices = {};

			for (size_t i = 0; i < 18; ++i)
			{
				indices[i] = i;
			}

			for (const auto& client_num : indices)
			{
				const auto target_client = session->clients[client_num].activeClient;

				if (target_client)
				{
					ImGui::AlignTextToFramePadding();
					ImGui::TextUnformatted(std::to_string(client_num).data());

					ImGui::NextColumn();

					const auto player_xuid = target_client->fixedClientInfo.xuid;
					const auto player_name = target_client->fixedClientInfo.gamertag;

					ImGui::PushStyleColor(ImGuiCol_Text, player_xuid == game::LiveUser_GetXuid(0)
						? ImColor(0, 255, 127, 250).Value : ImColor(200, 200, 200, 250).Value);

					ImGui::AlignTextToFramePadding();

					const auto selected = ImGui::Selectable((player_name + "##"s + std::to_string(client_num)).data());

					ImGui::PopStyleColor();

					if (const auto f = friends::get(player_xuid); f && f->id == player_xuid)
					{
						ImGui::SameLine(0, spacing);

						auto friend_name = "(" + f->name + ")";
						if (f->name == player_name)
							friend_name = "[Friend]";

						ImGui::TextColored(ImVec4(0.0f, 0.5f, 1.0f, 0.95f), "%s", friend_name.data());
					}

					const auto popup = "player_popup##" + std::to_string(client_num);

					if (selected)
						ImGui::OpenPopup(popup.data());

					if (ImGui::BeginPopup(popup.data(), ImGuiWindowFlags_NoMove))
					{
						if (ImGui::BeginMenu(player_name + "##"s + std::to_string(client_num) + "player_menu"))
						{
							ImGui::MenuItem(player_name + "##"s + std::to_string(client_num) + "player_menu_item", nullptr, false, false);

							if (ImGui::IsItemClicked())
							{
								ImGui::LogToClipboardUnformatted(player_name);
							}

							if (ImGui::IsItemHovered())
							{
								ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
							}

							if (ImGui::BeginMenu("Add to friends list##" + std::to_string(client_num)))
							{
								auto friend_player_name = std::string{ player_name };

								ImGui::InputTextWithHint("##" + std::to_string(client_num), "Name", &friend_player_name);

								if (ImGui::Button("Add friend"))
								{
									friends::friends.emplace_back(friends::friend_info{ player_xuid, friend_player_name });
									friends::write();
								}

								ImGui::EndMenu();
							}
							
							ImGui::EndMenu();
						}

						ImGui::Separator(); 
						
						const auto platform_type = target_client->fixedClientInfo.dwPlatformType;
						auto platform_name = "Invalid: " + std::to_string(platform_type); // 0x7FF7DD0697C0

						switch (platform_type)
						{
						case 2:
							platform_name = "Battle.net";
							break;
						case 6:
							platform_name = "Playstation 4";
							break;
						case 7:
							platform_name = "Playstation 5";
							break;
						case 17:
							platform_name = "Xbox One";
							break;
						case 25:
							platform_name = "Xbox Series X";
							break;
						default:
							break;
						}

						if (ImGui::MenuItem(platform_name))
						{
							ImGui::LogToClipboardUnformatted(platform_name);
						}

						const auto input_device = target_client->mutableClientInfo.inputDevice ? "Controller" : "Mouse";

						if (ImGui::MenuItem(input_device))
						{
							ImGui::LogToClipboardUnformatted(input_device);
						}

						ImGui::Separator();
						
						if (ImGui::MenuItem(std::to_string(player_xuid)))
						{
							ImGui::LogToClipboardUnformatted(std::to_string(player_xuid));
						}

						const auto netadr = target_client->sessionInfo[session->type].netAdr;
						
						game::XNADDR xn;
						game::dwNetadrToCommonAddr(netadr, &xn, sizeof xn, nullptr);

						const auto is_netadr_valid = netadr.inaddr && netadr.type != game::NA_BAD;
						const auto ip_string = is_netadr_valid ? xn.to_string() : "Invalid IP Data";

						if (ImGui::MenuItem(ip_string))
						{
							ImGui::LogToClipboardUnformatted(ip_string);
						}

						ImGui::Separator();

						if (ImGui::BeginMenu("Exploits##" + std::to_string(client_num)))
						{
							if (ImGui::MenuItem("Crash", nullptr, nullptr, is_netadr_valid))
							{
								exploit::send_crash(netadr, player_xuid);
							}
							
							if (ImGui::MenuItem("Show migration screen", nullptr, nullptr, is_netadr_valid))
							{
								exploit::send_mstart_packet(netadr);
							}

							if (ImGui::MenuItem("Kick from lobby", nullptr, nullptr, is_netadr_valid))
							{
								exploit::send_connect_response_migration_packet(netadr);
							}
							
							if (ImGui::MenuItem("Disconnect client from lobby"))
							{
								exploit::lobby_msg::send_disconnect_client(player_xuid);
							}

							if (ImGui::BeginMenu("Send OOB##" + std::to_string(client_num), is_netadr_valid))
							{
								static auto string_input = ""s;

								ImGui::InputTextWithHint("##" + std::to_string(client_num), "OOB", &string_input);

								if (ImGui::MenuItem("Send OOB", nullptr, nullptr, !string_input.empty()))
								{
									game::net::oob::send(netadr, string_input);
								}

								ImGui::EndMenu();
							}
							
							ImGui::EndMenu();
						}

						ImGui::EndPopup();
					}

					ImGui::NextColumn();

					if (ImGui::GetColumnIndex() == 0)
					{
						ImGui::Separator();
					}
				}
			}
			
			ImGui::EndColumns();
			ImGui::EndTabItem();
		}
	}

	void draw()
	{
		if (menu::open)
		{
			ImGui::SetNextWindowSize({ 480, 480 }, ImGuiCond_Once);

			if (!ImGui::Begin(window_title, nullptr, window_flags))
			{
				ImGui::End();
				return;
			}

			if (ImGui::BeginTabBar("Tabs"))
			{
				ImGui::Separator();

				const auto width = ImGui::GetContentRegionAvail().x;
				const auto spacing = ImGui::GetStyle().ItemInnerSpacing.x;
				
				if (ImGui::BeginTabItem("Misc"))
				{
					ImGui::Checkbox("Log out-of-band packets", &events::connectionless_packet::log_commands);
					ImGui::Checkbox("Log instant messages", &events::instant_message::log_messages);
					ImGui::Checkbox("Log lobby messages", &events::lobby_msg::log_messages);
					ImGui::Checkbox("Prevent join", &events::prevent_join);
					
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Server"))
				{
					static auto target_id_input{ ""s };

					ImGui::SetNextItemWidth(width * 0.85f);
					ImGui::InputTextWithHint("##target_steam_id", "Steam ID", &target_id_input);

					const auto target_id{ std::strtoull(target_id_input.data(), nullptr, 10) };

					if (ImGui::MenuItem("Send popup", nullptr, nullptr, target_id && !target_id_input.empty()))
					{
						scheduler::once([=]()
						{
							exploit::instant_message::send_popup(target_id);
						}, scheduler::pipeline::main); 
					}
					
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Extras"))
				{
					if (begin_section("Execute command"))
					{
						static auto command_input = ""s;

						ImGui::SetNextItemWidth(width * 0.85f);
						ImGui::InputTextWithHint("##command_input", "Command", &command_input);

						ImGui::SameLine();

						if (ImGui::Button("Execute##execute_command", { -1, 0.0f }))
						{
							command::execute(command_input);
						}
					}

					if (begin_section("Join game via ID"))
					{
						static auto id_input = ""s;

						ImGui::SetNextItemWidth(width * 0.85f);
						ImGui::InputTextWithHint("##id_input", "ID", &id_input);

						ImGui::SameLine();

						if (ImGui::Button("Join##join_via_id", { -1, 0.0f }))
						{
							command::execute("join " + id_input);
						}
					}
					
					ImGui::EndTabItem();
				}

				menu::draw_player_list(width, spacing);
				friends::draw_friends_list(width, spacing);
			}
		}
	}
}