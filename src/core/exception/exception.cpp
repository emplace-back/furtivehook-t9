#include "dependencies/stdafx.hpp"
#include "exception.hpp"

namespace exception
{
	utils::hook::detour rtl_dispatch_exception_hook; 
	std::mutex exception_mutex;
	
	namespace
	{
		using callback = std::function<bool(CONTEXT&)>;
		std::vector<callback> callbacks;

		void on_exception(const callback& cb)
		{
			callbacks.push_back(cb);
		}
		
		bool handle(PEXCEPTION_POINTERS ex)
		{
			auto& ctx = *ex->ContextRecord; 
			const auto code = ex->ExceptionRecord->ExceptionCode;
			const auto addr = ctx.Rip;
			const auto base = game::get_base();

			if (utils::nt::is_shutdown_in_progress())
				return false;
			
			if (code == STATUS_ILLEGAL_INSTRUCTION || code == STATUS_PRIVILEGED_INSTRUCTION || code < STATUS_ACCESS_VIOLATION || code == 0xe06d7363)
				return false;
			
			if (ex->ExceptionRecord->NumberParameters > 1 && ex->ExceptionRecord->ExceptionInformation[1] == 0xFFFFFFFFFFFFFFFF)
				return false;

			if (std::any_of(callbacks.begin(), callbacks.end(), [&](const auto& callback) { return callback(ctx); }))
				return true;

			if (dvars::handle_exception(ex))
				return true;

			const std::lock_guard<std::mutex> _(exception_mutex);

			const auto game = utils::nt::library{};
			const auto source_module = utils::nt::library::get_by_address(addr);

			const auto module_name = !source_module.get_name().empty() ? source_module.get_name() : "N/A";

			std::vector<uint8_t*> callstack;

			for (size_t i = 0; i < 0x80 && callstack.size() < 0x20; ++i)
			{
				const auto addr = *reinterpret_cast<uint8_t**>(ctx.Rsp + sizeof(uint64_t) + i);

				if (utils::nt::library::get_by_address(addr))
					callstack.push_back(addr);
			}

			auto message = utils::string::va("Exception: 0x%08X at 0x%llX", code, source_module == game ? game::derelocate(addr) : addr);

			if (source_module)
				message += utils::string::va("\n%s: 0x%llX", module_name.data(), source_module.get_ptr());

			if (!callstack.empty())
			{
				message += "\n\nStack trace:\n";

				for (const auto& stack : callstack)
				{
					const auto stack_module = utils::nt::library::get_by_address(stack);
					const auto stack_addr = stack_module == game ? game::derelocate(stack) : uintptr_t(stack);
					
					message += utils::string::va("0x%llX", stack_addr);

					if (const auto module_name = stack_module.get_name(); !module_name.empty())
						message += " (" + module_name + ")";

					message += "\n";
				}

				message.resize(message.size() - 1);
			}

			message += "\n\nRegisters:\n";

			const static std::vector<const char*> registers =
			{
				"rax",
				"rcx",
				"rdx",
				"rbx",
				"rsp",
				"rbp",
				"rsi",
				"rdi",
				"r8",
				"r9",
				"r10",
				"r11",
				"r12",
				"r13",
				"r14",
				"r15",
			};

			for (size_t i = 0; i < registers.size(); ++i)
			{
				auto value = *(&ctx.Rax + i);
				
				if (utils::nt::library::get_by_address(value) == game)
					value = game::derelocate(value);
				
				message += utils::string::va("%s: 0x%llX\n", registers[i], value);
			}

			message.resize(message.size() - 1);
			
			PRINT_LOG("%s", message.data());
			MessageBoxA(nullptr, message.data(), "Exception", MB_ICONERROR);
			utils::nt::terminate(code);

			return false;
		}

		bool __stdcall rtl_dispatch_exception(PEXCEPTION_RECORD ex, PCONTEXT ctx)
		{
			// KiUserExceptionDispatcher
			if ((*reinterpret_cast<uint64_t*>(_ReturnAddress()) & 0xFFFFFFFFFFFFFF) == 0xCC8B480C74C084)
			{
				auto& pex = EXCEPTION_POINTERS
				{ 
					ex, 
					ctx 
				};
				
				if (exception::handle(&pex))
					return true;
			}

			return rtl_dispatch_exception_hook.call<bool>(ex, ctx);
		}
	}

	void initialize()
	{
		const auto ki_user_exception_dispatcher = utils::nt::library("ntdll.dll").get_proc<uint8_t*>("KiUserExceptionDispatcher");
		
		if (!ki_user_exception_dispatcher)
			return;

		rtl_dispatch_exception_hook.create(utils::hook::extract(ki_user_exception_dispatcher + 0x29 + 1), rtl_dispatch_exception);

		return;
		dvars::initialize();

		exception::on_exception([](auto& ctx)
		{
			const auto instr = *reinterpret_cast<uint64_t*>(ctx.Rip);

			if ((instr & 0xFFFFFFFFFF) == 0x48B548B44)
			{
				static int items[2] =
				{
					0,
					std::numeric_limits<uint16_t>::max()
				};

				ctx.Rax = reinterpret_cast<uintptr_t>(&items);
				ctx.Rip += 0x24;

				return true;
			}
			else if ((instr & 0xFFFFFFFFFFFF) == 0xC73B1041B60F)
			{
				ctx.R8 = 0;
				ctx.Rip -= 0xB;

				return true;
			}

			return false;
		});
		
		scheduler::loop([]()
		{
			const auto inventory = uintptr_t(reinterpret_cast<uintptr_t*>(OFFSET(offsets::playerInventory)));

			if (!inventory)
				return;

			if (!*reinterpret_cast<bool*>(inventory + 0x3B0))
				return; 
			
			if (*reinterpret_cast<int*>(inventory + 0x61E3C) != 4)
				return;

			auto items = reinterpret_cast<int*>(inventory + 0x3B0);
			auto num = &items[0x186A1];

			constexpr auto int_max = std::numeric_limits<int>::max();

			if (*num == int_max)
				return;

			*num = int_max;
		}, scheduler::pipeline::main);
	}
}