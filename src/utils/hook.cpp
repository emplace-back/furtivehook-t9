#include "dependencies/std_include.hpp"
#include "hook.hpp"

namespace utils::hook
{
	namespace
	{
		[[maybe_unused]] class _
		{
		public:
			_()
			{
				if (MH_Initialize() != MH_OK)
				{
					throw std::runtime_error("Failed to initialize MinHook");
				}
			}

			~_()
			{
				MH_Uninitialize();
			}
		} __;
	}

	void assembler::pushad64()
	{
		this->push(rax);
		this->push(rcx);
		this->push(rdx);
		this->push(rbx);
		this->push(rsp);
		this->push(rbp);
		this->push(rsi);
		this->push(rdi);

		this->sub(rsp, 0x40);
	}

	void assembler::popad64(const bool ret_val)
	{
		this->add(rsp, 0x40);

		this->pop(rdi);
		this->pop(rsi);
		this->pop(rbp);
		this->pop(rsp);
		this->pop(rbx);
		this->pop(rdx);
		this->pop(rcx);
		if (ret_val) this->add(rsp, 0x8);
		else this->pop(rax);
	}

	void assembler::prepare_stack_for_call()
	{
		const auto reserve_callee_space = this->newLabel();
		const auto stack_unaligned = this->newLabel();

		this->test(rsp, 0xF);
		this->jnz(stack_unaligned);

		this->sub(rsp, 0x8);
		this->push(rsp);

		this->push(rax);
		this->mov(rax, ptr(rsp, 8, 8));
		this->add(rax, 0x8);
		this->mov(ptr(rsp, 8, 8), rax);
		this->pop(rax);

		this->jmp(reserve_callee_space);

		this->bind(stack_unaligned);
		this->push(rsp);

		this->bind(reserve_callee_space);
		this->sub(rsp, 0x40);
	}

	void assembler::restore_stack_after_call()
	{
		this->lea(rsp, ptr(rsp, 0x40));
		this->pop(rsp);
	}

	asmjit::Error assembler::call(void* target)
	{
		return Assembler::call(uintptr_t(target));
	}

	asmjit::Error assembler::jmp(void* target)
	{
		return Assembler::jmp(uintptr_t(target));
	}

	detour::detour(const uintptr_t place, void* target) : detour(reinterpret_cast<void*>(place), target)
	{
	}

	detour::detour(void* place, void* target)
	{
		this->create(place, target);
	}

	detour::~detour()
	{
		this->clear();
	}

	void detour::enable()
	{
		MH_EnableHook(this->place);

		if (!this->moved_data.empty())
		{
			this->move();
		}
	}

	void detour::disable()
	{
		this->un_move();
		MH_DisableHook(this->place);
	}

	void detour::create(void* place, void* target)
	{
		this->clear();
		this->place = place;

		if (MH_CreateHook(this->place, target, &this->original) != MH_OK)
		{
			throw std::runtime_error(string::va("Unable to create hook at location: 0x%llX", this->place));
		}

		this->enable();
	}

	void detour::create(const uintptr_t place, void* target)
	{
		this->create(reinterpret_cast<void*>(place), target);
	}

	void detour::clear()
	{
		if (this->place)
		{
			this->un_move();
			MH_RemoveHook(this->place);
		}

		this->place = nullptr;
		this->original = nullptr;
		this->moved_data = {};
	}

	void detour::move()
	{
		this->moved_data = move_hook(this->place);
	}

	void detour::un_move()
	{
		if (!this->moved_data.empty())
		{
			copy(this->place, this->moved_data.data(), this->moved_data.size());
		}
	}
	
	class memory
	{
	public:
		memory() = default;

		memory(const uintptr_t address) : memory()
		{
			this->length_ = 0x1000;
			this->buffer_ = allocate_somewhere_near(address, this->length_);
			
			if (!this->buffer_)
			{
				throw std::runtime_error("Failed to allocate");
			}
		}

		~memory()
		{
			if (this->buffer_)
			{
				VirtualFree(this->buffer_, 0, MEM_RELEASE);
			}
		}

		memory(memory&& obj) noexcept : memory()
		{
			this->operator=(std::move(obj));
		}

		memory& operator=(memory&& obj) noexcept
		{
			if (this != &obj)
			{
				this->~memory();
				this->buffer_ = obj.buffer_;
				this->length_ = obj.length_;
				this->offset_ = obj.offset_;

				obj.buffer_ = nullptr;
				obj.length_ = 0;
				obj.offset_ = 0;
			}

			return *this;
		}

		void* allocate(const size_t length)
		{
			if (!this->buffer_)
			{
				return nullptr;
			}

			if (this->offset_ + length > this->length_)
			{
				return nullptr;
			}

			const auto ptr = this->get_ptr();
			this->offset_ += length;
			return ptr;
		}

		void* get_ptr() const
		{
			return this->buffer_ + this->offset_;
		}

	private:
		uint8_t* buffer_{};
		size_t length_{};
		size_t offset_{};
	};
	
	void* get_memory_near(const uintptr_t address, const size_t size)
	{
		static concurrency::container<std::vector<memory>> memory_container{};

		return memory_container.access<void*>([&](std::vector<memory>& memories)
		{
			for (auto& memory : memories)
			{
				if (is_relatively_far(address, memory.get_ptr()))
					continue;
				
				const auto buffer = memory.allocate(size);
				if (buffer)
				{
					return buffer;
				}
			}

			memories.emplace_back(address);
			return memories.back().allocate(size);
		});
	}
	
	uint8_t* allocate_somewhere_near(const uintptr_t base_address, const size_t size)
	{
		size_t offset{ 0 };
		
		while (true)
		{
			offset += size;
			
			const auto* target_address = reinterpret_cast<const uint8_t*>(base_address) - offset;
			
			if (is_relatively_far(base_address, target_address))
				return nullptr;

			const auto result = VirtualAlloc(const_cast<uint8_t*>(target_address), size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			
			if (result)
			{
				if (!is_relatively_far(base_address, target_address))
					return static_cast<uint8_t*>(result); 

				VirtualFree(result, 0, MEM_RELEASE);
				return nullptr;
			}
		}
	}
	
	void retn(const uintptr_t address)
	{
		set(address, instr::ret);
	}
	
	void nop(const uintptr_t address, const size_t size)
	{
		DWORD old_protect{ 0 };
		VirtualProtect(reinterpret_cast<void*>(address), size, PAGE_EXECUTE_READWRITE, &old_protect);

		std::memset(reinterpret_cast<void*>(address), std::uint8_t(instr::nop), size);

		VirtualProtect(reinterpret_cast<void*>(address), size, old_protect, &old_protect);

		FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(address), size);
	}

	void nop(const void* place, const size_t size)
	{
		return nop(uintptr_t(place), size);
	}

	std::vector<uint8_t> move_hook(const uintptr_t address)
	{
		std::vector<uint8_t> original_data{};

		const auto data = reinterpret_cast<uint8_t*>(address);

		if (*data == 0xE9)
		{
			original_data.insert(original_data.begin(), data, data + 6);

			const auto target = follow_branch(data);
			nop(data, 1);
			jump(data + 1, target);
		}
		else if (*data == 0xFF && data[1] == 0x25)
		{
			original_data.insert(original_data.begin(), data, data + 15);

			copy(data + 1, data, 14);
			nop(data, 1);
		}
		else
		{
			throw std::runtime_error("No branch instruction found");
		}

		return original_data;
	}

	std::vector<uint8_t> move_hook(const void* pointer)
	{
		return move_hook(uintptr_t(pointer));
	}

	void* assemble(const std::function<void(assembler&)>& asm_function)
	{
		static asmjit::JitRuntime runtime;

		asmjit::CodeHolder code;
		code.init(runtime.environment());

		assembler a(&code);

		asm_function(a);

		void* result = nullptr;
		runtime.add(&result, &code);

		return result;
	}

	void* follow_branch(void* address)
	{
		const auto data = static_cast<uint8_t*>(address);

		if (*data != 0xE8 && *data != 0xE9)
		{
			throw std::runtime_error("No branch instruction found");
		}

		return extract<void*>(data + 1);
	}
}