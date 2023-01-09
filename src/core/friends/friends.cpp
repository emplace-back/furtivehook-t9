#include "dependencies/std_include.hpp"
#include "friends.hpp"

namespace friends
{
	bool fetch{ false };
	std::vector<friend_info> friends; 
	uint32_t NONCE{ 0x696969 };

	namespace
	{
		std::string get_friends_file()
		{
			const utils::nt::library self{};
			return self.get_folder() + "\\furtivehook\\friends.json";
		}

		json load_friends()
		{
			std::string data{};
			const auto friends = get_friends_file();
			if (!utils::io::read_file(friends, &data))
			{
				return {};
			}

			const auto result = json::parse(data);
			if (result.empty() || !result.is_object())
			{
				return {};
			}

			return result;
		}

		void fetch_sessions(const std::vector<std::uint64_t>& targets)
		{
			if (!fetch)
				return;

			std::vector<uint64_t> online_targets;
			online_targets.reserve(targets.size());

			for (const auto& id : targets)
			{
				if (const auto f = friends::get(id))
				{
					online_targets.emplace_back(id); 
				}
			}

			events::instant_message::send_info_request(online_targets, friends::NONCE);
		}

		void update_online_status()
		{
			static std::vector<std::uint64_t> recipients{};

			if (recipients.empty())
			{
				for (auto& f : friends)
				{
					if (f.is_online() && std::time(nullptr) - f.last_online > 15)
					{
						f.response = {};
						friends::write();
					}
					
					recipients.emplace_back(f.id);
				}
			}

			if (recipients.empty())
				return;

			constexpr auto interval{ 5s };
			const auto now{ std::chrono::high_resolution_clock::now() };
			static std::chrono::high_resolution_clock::time_point last_time{};

			if (last_time + interval >= now)
				return;

			constexpr auto num{ 16 }; 
			
			static size_t batch_index{ 0 };

			if (batch_index * num < recipients.size())
			{
				const std::vector<uint64_t> batch
				{
					recipients.begin() + (batch_index * num),
					recipients.begin() + std::min(recipients.size(), (batch_index * num) + num)
				};

				friends::fetch_sessions(batch);

				++batch_index;
			}
			else
			{
				last_time = now; 
				batch_index = 0;
				recipients.clear();
			}
		}
		
		void read()
		{
			friends.clear();

			const auto json = load_friends();
			for (const auto& data : json)
			{
				for (const auto& element : data)
				{
					auto& info = element.get<friend_info>();
					info.response = {};

					friends.emplace_back(std::move(info));
				}
			}
		}

		void remove(const std::uint64_t id)
		{
			friends.erase(std::remove_if(friends.begin(), friends.end(), [&id](const auto& f) { return f.id == id; }));
			friends::write();
		}
	}

	friend_info* get(const uint64_t id)
	{
		const auto entry = std::find_if(friends.begin(), friends.end(), [id](const auto& f) { return f.id == id; });

		if (entry != friends.end())
		{
			return &*entry;
		}

		return nullptr;
	}
	
	void write()
	{
		json result{};
		result["friends"] = friends;

		const auto friends = get_friends_file();
		utils::io::write_file(friends, result.dump());
	}

	void draw_friends_list(const float width, const float spacing)
	{
		if (ImGui::BeginTabItem("Friends"))
		{
			ImGui::Checkbox("Fetch sessions", &friends::fetch); 
			
			static ImGuiTextFilter filter;
			ImGui::TextUnformatted("Search friends");
			filter.Draw("##search_friend", "Name", width * 0.85f);

			ImGui::SameLine(0.0f, spacing);

			const auto popup = "Add friend##add_friend_popup"s; 
			
			if (ImGui::Button("Add friend"))
			{
				ImGui::OpenPopup(popup.data());
			}

			if (ImGui::BeginPopupModal(popup.data(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				static auto name_input = ""s;
				static auto id_input = ""s;

				ImGui::SetNextItemWidth(width * 0.5f);
				ImGui::InputTextWithHint("##name_input", "Name", &name_input);
				
				ImGui::SetNextItemWidth(width * 0.5f);
				ImGui::InputTextWithHint("##id_input", "ID", &id_input);

				if (ImGui::MenuItem("Add friend", nullptr, nullptr, !name_input.empty() && !id_input.empty()))
				{
					friends.emplace_back(friend_info{ std::strtoull(id_input.data(), nullptr, 10), name_input });
					friends::write();

					ImGui::CloseCurrentPopup();
				}

				if (ImGui::IsKeyPressedWithIndex(ImGuiKey_Escape))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::Separator();
			
			ImGui::BeginColumns("Friends", 2, ImGuiColumnsFlags_NoResize);

			ImGui::TextUnformatted("Friend");
			ImGui::NextColumn();
			ImGui::TextUnformatted("Online Status");
			ImGui::NextColumn();

			ImGui::Separator();

			std::vector<friend_info> sorted_friends{ friends };
			std::sort(sorted_friends.begin(), sorted_friends.end());

			for (const auto& f : sorted_friends)
			{
				if (!filter.PassFilter(f.name))
					continue;
				
				const auto xuid = std::to_string(f.id);
				const auto label = f.name + "##friend" + xuid;
				
				ImGui::AlignTextToFramePadding(); 
				
				const auto selected = ImGui::Selectable(label);
				
				const auto popup = "friend_popup##" + xuid;
				
				if (selected)
					ImGui::OpenPopup(popup.data());

				if (ImGui::BeginPopup(popup.data(), ImGuiWindowFlags_NoMove))
				{
					ImGui::MenuItem(f.name + "##friend_menu_item" + xuid, nullptr, false, false);

					if (ImGui::MenuItem("Remove"))
					{
						friends::remove(f.id);
					}
					
					if (ImGui::BeginMenu("Rename##" + xuid))
					{
						auto& name = friends::get(f.id)->name;

						if (ImGui::InputTextWithHint("##" + xuid, "Name", &name))
						{
							friends::write();
						}

						ImGui::EndMenu();
					}
					
					ImGui::Separator();

					if (ImGui::MenuItem(xuid))
					{
						ImGui::LogToClipboardUnformatted(xuid);
					}

					const auto response = f.response;
					const auto party_session = response.lobby[0];
					const auto lobby_session = response.lobby[1];
					game::netadr_t party_netadr{};
					
					if (party_session.isValid)
					{
						ImGui::Separator();

						auto message{ "Party: " + party_session.serializedAdr.xnaddr.to_string() };
						message.append(" - "s + party_session.hostName + " (" + std::to_string(party_session.hostXuid) + ")");

						if (ImGui::MenuItem(message))
						{
							ImGui::LogToClipboardUnformatted(message);
						}
					}

					if (lobby_session.isValid && lobby_session.hostXuid != f.id)
					{
						const auto message{ "Session: " + party_session.serializedAdr.xnaddr.to_string() };
						
						if (ImGui::MenuItem(message))
						{
							ImGui::LogToClipboardUnformatted(message);
						}
					}
					
					ImGui::Separator();

					if (ImGui::MenuItem("Join session"))
					{
						command::execute("join " + xuid);
					}

					ImGui::Separator();

					const auto is_ready{ f.is_online() && party_netadr.inaddr };

					if (ImGui::MenuItem("Test", nullptr, nullptr, is_ready)) {}

					ImGui::EndPopup();
				}

				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();

				const auto is_online = f.is_online();
				const auto timestamp = f.last_online ? utils::string::data_time(f.last_online, false) : "Never";
				const auto online_status = is_online ? "Online" : "Last seen: " + timestamp;
				
				ImGui::TextColored(is_online ? ImColor(0, 255, 127, 250).Value : ImColor(200, 200, 200, 250).Value, online_status.data());

				ImGui::NextColumn();

				if (ImGui::GetColumnIndex() == 0)
				{
					ImGui::Separator();
				}
			}

			ImGui::EndColumns();
			ImGui::EndTabItem();
		}
	}

	void initialize()
	{
		scheduler::on_game_initialized([]()
		{
			scheduler::once(update_online_status, scheduler::pipeline::main);
			scheduler::loop(update_online_status, scheduler::pipeline::main);
		}, scheduler::pipeline::main);

		try
		{
			friends::read();
			friends::write();
		}
		catch (const std::exception& ex)
		{
			PRINT_LOG("Could not read 'friends.json' (%s)", ex.what());
		}
	}
}
