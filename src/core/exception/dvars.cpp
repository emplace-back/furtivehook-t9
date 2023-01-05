#include "dependencies/std_include.hpp"
#include "exception.hpp"

namespace exception::dvars
{
	namespace
	{
		using callback = std::function<void(CONTEXT&)>; 
		std::unordered_map<int, callback>& get_callbacks()
		{
			static std::unordered_map<int, callback> callbacks{};
			return callbacks;
		}

		void register_hook(const hook_dvar index, const uintptr_t address, const callback& callback)
		{
			const auto* dvar = *reinterpret_cast<uintptr_t**>(address);

			get_callbacks()[index] = [=](auto& ctx)
			{
				ctx.Rcx = reinterpret_cast<uintptr_t>(dvar);
				callback(ctx);
			};

			utils::hook::set<uintptr_t>(address, index);
		}
	}

	bool handle_exception(PEXCEPTION_POINTERS ex)
	{
		if (ex->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
			return false;

		const auto& callbacks = get_callbacks();
		const auto handler = callbacks.find(ex->ContextRecord->Rcx);

		if (handler == callbacks.end())
			return false;

		handler->second(*ex->ContextRecord);
		return true;
	}

	void initialize()
	{
		dvars::register_hook(hook_dvar::handle_packet, OFFSET(offsets::lobbymsg_prints),
			[](auto& ctx)
		{
			const auto stack{ ctx.Rsp + sizeof(uint64_t) + 0x40 };
			const auto ret_address{ *reinterpret_cast<uintptr_t*>(stack) };

			if (ret_address == OFFSET(offsets::ret_handle_packet_internal))
			{
				const auto msg = reinterpret_cast<game::msg_t*>(stack + sizeof(uint64_t) + 0x40);
				const auto msg_backup = *msg;

				if (events::lobby_msg::handle_packet(
					*reinterpret_cast<game::LobbyModule*>(stack + sizeof(uint64_t) + 0xE0),
					*reinterpret_cast<game::netadr_t*>(ctx.Rsi),
					*msg))
				{
					msg->type = static_cast<game::MsgType>(game::MESSAGE_TYPE_NONE);
				}
				else
				{
					*msg = msg_backup;
				}
			}
		});
	}
}