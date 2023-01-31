#include "dependencies/stdafx.hpp"
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
		const auto lobbymsg_prints_ptr = utils::hook::scan_pattern(signatures::lobbymsg_prints_ptr);
		const auto unlockables_ptr = utils::hook::scan_pattern(signatures::unlockables_ptr);
		const auto netchan_debugspew_ptr = utils::hook::scan_pattern(signatures::netchan_debugspew_ptr);

		if (!lobbymsg_prints_ptr || !unlockables_ptr || !netchan_debugspew_ptr)
			return;

		/*dvars::register_hook(hook_dvar::unlockables, utils::hook::extract<uintptr_t>(unlockables_ptr + 3),
			[](auto& ctx)
		{
			const auto retaddr = *reinterpret_cast<uintptr_t*>(ctx.Rsp + sizeof(uint64_t) + 0x40);

			const static std::vector<std::pair<uintptr_t, uintptr_t>> unlock_handlers =
			{
				{ 0xC0D8B481074C084, 0x7C }, // BG_UnlockablesIsItemLockedFromBuffer
				{ 0xA0D8B481074C084, 0xDD }, // BG_UnlockablesIsItemAttachmentLockedFromBuffer
			};

			const auto unlock = std::find_if(unlock_handlers.begin(), unlock_handlers.end(), [=](const auto& handler) 
			{ 
				return (*reinterpret_cast<uint64_t*>(retaddr) & 0xFFFFFFFFFFFFFFF) == handler.first;
			});

			if (unlock != unlock_handlers.end())
			{
				// clean up Dvar_GetBool
				ctx.Rsp += 0x48 + sizeof(uint64_t);
				ctx.Rip = retaddr + unlock->second;
			}
		});*/ 
		
		dvars::register_hook(hook_dvar::netchan_debugspew, utils::hook::extract<uintptr_t>(netchan_debugspew_ptr + 3),
			[](auto& ctx)
		{
			const auto stack = ctx.Rsp + sizeof(uint64_t) + 0x40;
			const auto retaddr = *reinterpret_cast<uintptr_t*>(stack);

			// Netchan_GetMessage
			if (*reinterpret_cast<uint64_t*>(retaddr) == 0xBE0D8B480C75C084)
			{
				const auto channel = static_cast<game::NetChanMsgType>(ctx.R15 / sizeof(uint64_t));
				const auto chan = reinterpret_cast<game::NetChanMessage_s*>(ctx.Rbx);
				const auto msg = reinterpret_cast<game::msg_t*>(ctx.Rsi);
				
				auto msg_backup = *msg;
				auto handled = false;

				if (channel == game::NETCHAN_SNAPSHOT || channel == game::NETCHAN_CLIENTMSG)
				{
					if (*reinterpret_cast<int*>(msg->data) == -1)
					{
						msg->read<int>();
						handled = events::connectionless_packet::handle_command(chan->destAddress, msg, channel);
					}
				}
				else if (channel == game::NETCHAN_CLIENT_CMD || channel == game::NETCHAN_CONNECTIONLESS_CMD)
				{
					handled = events::connectionless_packet::handle_command(chan->destAddress, msg, !(channel & 1));
				}
				else
				{
					handled = events::lobby_msg::handle(chan->destAddress, msg, channel);
				}

				if (handled)
					msg->cursize = 0;
				else
					*msg = msg_backup;
			}
		});
		
		dvars::register_hook(hook_dvar::handle_packet, utils::hook::extract<uintptr_t>(lobbymsg_prints_ptr + 3),
			[](auto& ctx)
		{
			const auto stack = ctx.Rsp + sizeof(uint64_t) + 0x40;
			const auto retaddr = *reinterpret_cast<uintptr_t*>(stack);

			// LobbyMsgRW_PrintDebugMessage
			if (*reinterpret_cast<uint64_t*>(retaddr) == 0x357B801F7411FF83)
			{
				// clean up Dvar_GetBool
				ctx.Rsp += 0x48 + sizeof(uint64_t); 
				ctx.Rax = static_cast<DWORD64>(-1);
				ctx.Rip = retaddr + 0x38;
			}
		});
	}
}