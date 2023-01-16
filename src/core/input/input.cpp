#include "dependencies/std_include.hpp"
#include "input.hpp"

namespace input
{
	utils::hook::detour translate_message_hook; 
	utils::hook::detour dispatch_hook_a_hook;
	utils::hook::detour get_raw_input_buffer_hook;
	std::vector<hotkey_t> registered_keys;
	HWND hwnd_;
	WNDPROC wndproc_;

	void on_key(UINT key, void(*cb)())
	{
		registered_keys.emplace_back(hotkey_t{ key, cb });
	}

	bool is_key_down(UINT key, UINT msg, WPARAM wparam)
	{
		return msg == WM_KEYDOWN && wparam == key;
	}

	bool was_key_pressed(UINT key, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		return input::is_key_down(key, msg, wparam) && !(lparam >> 0x1E);
	}

	bool should_ignore_msg(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		const auto ignore = msg == WM_ACTIVATE || msg == WM_ACTIVATEAPP || (msg >= WM_KEYFIRST && msg <= WM_MOUSELAST && msg != WM_SYSCOMMAND);
		return (menu::open && ignore) || input::was_key_pressed(menu::hotkey, msg, wparam, lparam);
	}

	bool handle(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		ImGui_ImplWin32_WndProcHandler(hwnd_, msg, wparam, lparam);
		
		for (const auto& hotkey : registered_keys)
		{
			if (menu::open && hotkey.key != menu::hotkey)
				continue;

			if (input::was_key_pressed(hotkey.key, msg, wparam, lparam))
				hotkey.func();
		}

		return input::should_ignore_msg(msg, wparam, lparam);
	}

	BOOL translate_message(const MSG *msg)
	{
		const auto result = translate_message_hook.call<BOOL>(msg);

		if (input::handle(msg->message, msg->wParam, msg->lParam))
		{
			ZeroMemory(const_cast<MSG*>(msg), sizeof msg);
			return FALSE;
		}

		return result;
	}

	LRESULT dispatch_hook_a(int code, WPARAM wparam, LPARAM lparam, HOOKPROC fn)
	{
		const auto hook_id = static_cast<uint16_t>(code >> 0x10);

		if (hook_id == WH_KEYBOARD_LL)
		{
			const auto kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lparam);
			
			constexpr auto repeat_count = 1;
			const auto prev_state = wparam == WM_KEYUP;
			const auto transition = prev_state;

			const auto key_lparam = static_cast<LPARAM>(repeat_count
				| (kb->scanCode << 16)
				| ((kb->flags & 0x1) << 24)
				| ((kb->flags & 0x10) << 29)
				| (prev_state << 30)
				| (transition << 31));

			if (should_ignore_msg(static_cast<UINT>(wparam), static_cast<WPARAM>(kb->vkCode), key_lparam))
			{
				return 0;
			}
		}

		return dispatch_hook_a_hook.call<LRESULT>(code, wparam, lparam, fn);
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

	void initialize()
	{
		const auto dispatch_hook_a_ptr = utils::hook::scan_pattern("user32.dll", signatures::dispatch_hook_a_ptr);

		if (!dispatch_hook_a_ptr)
			return;
		
		dispatch_hook_a_hook.create(dispatch_hook_a_ptr, dispatch_hook_a);
		translate_message_hook.create(::TranslateMessage, translate_message);
		get_raw_input_buffer_hook.create(::GetRawInputBuffer, get_raw_input_buffer);
		
		input::on_key(menu::hotkey, menu::toggle);
	}
}