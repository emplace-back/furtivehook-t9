#include "dependencies/std_include.hpp"
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
				const auto target_client = session->get_client(client_num).activeClient;

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

					const auto popup = "player_popup##" + std::to_string(client_num);

					if (selected)
						ImGui::OpenPopup(popup.data());

					if (ImGui::BeginPopup(popup.data(), ImGuiWindowFlags_NoMove))
					{
						if (ImGui::BeginMenu(player_name + "##"s + std::to_string(client_num) + "player_menu"))
						{
							ImGui::MenuItem(player_name + "##"s + std::to_string(client_num) + "player_menu_item", nullptr, false, false);

							ImGui::EndMenu();
						}

						ImGui::Separator();
						
						if (ImGui::MenuItem(std::to_string(player_xuid)))
						{
							ImGui::LogToClipboardUnformatted(std::to_string(player_xuid));
						}

						const auto netadr = target_client->sessionInfo[session->type].netAdr;
						
						game::XNADDR xn;
						game::dwNetadrToCommonAddr(netadr, &xn, sizeof xn, nullptr);

						const auto ip_string = xn.to_string();

						if (ImGui::MenuItem(ip_string))
						{
							ImGui::LogToClipboardUnformatted(ip_string);
						}

						ImGui::Separator();

						if (ImGui::BeginMenu("Exploits##" + std::to_string(client_num)))
						{
							if (ImGui::MenuItem("Disconnect client from lobby"))
							{
								game::call(0x7FF6FE192890, 0, session->type, player_xuid, 2);
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
					ImGui::Checkbox("Log instant messages", &events::instant_message::log_messages);
					ImGui::Checkbox("Log lobby messages", &events::lobby_msg::log_messages); 
					ImGui::Checkbox("Prevent join", &events::prevent_join);
					
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Server"))
				{
					static auto target_steam_id{ ""s };

					ImGui::SetNextItemWidth(width * 0.85f);
					ImGui::InputTextWithHint("##target_steam_id", "Steam ID", &target_steam_id);

					const auto target_id{ std::strtoull(target_steam_id.data(), nullptr, 10) };

					if (ImGui::MenuItem("Send popup", nullptr, nullptr, target_id && !target_steam_id.empty()))
					{
						/*if (const auto clc = *reinterpret_cast<uintptr_t*>(OFFSET(0x7FF703CE4028)))
						{
							auto data{ "LM\n"s };
							const auto header{ 0x97Aui16 };
							data.append(reinterpret_cast<const char*>(&header), sizeof header);
							data.push_back(69);
							data.push_back(-1);

							game::call(0x7FF6FD6F7320, 4, *reinterpret_cast<game::netadr_t*>(clc + 16), data.data(), data.size());
						}*/

						scheduler::once([=]()
						{
							const auto message{ 0 };
							game::send_instant_message({ target_id }, 'f', &message, sizeof message);
						});
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

						if (ImGui::Button("Execute##execute_command", { 64.0f, 0.0f }))
						{
							game::Cmd_ExecuteSingleCommand(command_input);
						}
					}

					if (begin_section("Execute reliable command"))
					{
						static auto reliable_command_input = ""s;

						ImGui::SetNextItemWidth(width * 0.85f);
						ImGui::InputTextWithHint("##reliable_command_input", "Reliable Command", &reliable_command_input);

						ImGui::SameLine();

						if (ImGui::Button("Execute##execute_reliable_command", { 64.0f, 0.0f }))
						{
							game::CL_AddReliableCommand(0, reliable_command_input.data());
						}
					}
					
					ImGui::EndTabItem();
				}

				menu::draw_player_list(width, spacing);
			}
		}
	}
}