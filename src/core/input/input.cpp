#include "dependencies/std_include.hpp"
#include "input.hpp"

namespace input
{
	utils::hook::detour translate_message_hook; 
	utils::hook::detour get_raw_input_buffer_hook;
	utils::hook::detour set_windows_hook_ex_a_hook;
	std::vector<hotkey_t> registered_keys;
	HWND hwnd_;
	WNDPROC wndproc_;

	void on_key(UINT key, void(*cb)())
	{
		registered_keys.emplace_back(hotkey_t{ key, cb });
	}
	
	bool should_ignore_msg(UINT msg)
	{
		return msg == WM_ACTIVATE || msg == WM_ACTIVATEAPP || (msg >= WM_KEYFIRST && msg <= WM_MOUSELAST && msg != WM_SYSCOMMAND);
	}

	bool is_key_down(UINT key, UINT msg, WPARAM wparam)
	{
		return msg == WM_KEYDOWN && wparam == key;
	}

	bool was_key_pressed(UINT key, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		return is_key_down(key, msg, wparam) && !(lparam >> 0x1E);
	}

	bool handle(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		ImGui_ImplWin32_WndProcHandler(hwnd_, msg, wparam, lparam);
		
		for (const auto& hotkey : registered_keys)
		{
			if (menu::open && hotkey.key != menu::hotkey)
				continue;

			if (was_key_pressed(hotkey.key, msg, wparam, lparam))
				hotkey.func();
		}

		return (menu::open && should_ignore_msg(msg)) || was_key_pressed(menu::hotkey, msg, wparam, lparam);
	}

	BOOL translate_message(const MSG *msg)
	{
		const auto result = translate_message_hook.call<BOOL>(msg);

		if (input::handle(msg->message, msg->wParam, msg->lParam))
		{
			const_cast<MSG*>(msg)->hwnd = NULL;
			const_cast<MSG*>(msg)->message = WM_NULL;
			const_cast<MSG*>(msg)->wParam = 0;
			const_cast<MSG*>(msg)->lParam = 0;
			return FALSE;
		}

		return result;
	}

	UINT get_raw_input_buffer(PRAWINPUT data, PUINT psize, UINT size_header)
	{
		const auto result = get_raw_input_buffer_hook.call<UINT>(data, psize, size_header);

		if (menu::open)
		{
			ZeroMemory(data, *psize);
			return 0;
		}

		return result;
	}
	
	HHOOK set_windows_hook_ex_a(int id, HOOKPROC fn, HINSTANCE mod, DWORD thread_id)
	{
		if (id == WH_KEYBOARD_LL && utils::nt::library{}.get_handle() == mod)
		{
			return nullptr;
		}

		return set_windows_hook_ex_a_hook.call<HHOOK>(id, fn, mod, thread_id);
	}

	void initialize()
	{
		translate_message_hook.create(::TranslateMessage, translate_message);
		get_raw_input_buffer_hook.create(::GetRawInputBuffer, get_raw_input_buffer);
		set_windows_hook_ex_a_hook.create(::SetWindowsHookExA, set_windows_hook_ex_a);
		
		input::on_key(menu::hotkey, menu::toggle);
	}
}