#pragma once
#include "dependencies/stdafx.hpp"

#include <utils/asmjit/core/jitruntime.h>
#include <utils/asmjit/x86/x86assembler.h>

using namespace asmjit::x86;

namespace utils::hook
{
	enum class instr : uint8_t
	{
		call = 0xE8,
		jmp = 0xE9,
		ret = 0xC3,
		mov = 0xB8,
		nop = 0x90,
	};

	class assembler : public Assembler
	{
	public:
		using Assembler::Assembler;
		using Assembler::call;
		using Assembler::jmp;

		void pushad64();
		void popad64(const bool ret_val = false);

		void prepare_stack_for_call();
		void restore_stack_after_call();

		template <typename T>
		void call_aligned(T&& target)
		{
			this->prepare_stack_for_call();
			this->call(std::forward<T>(target));
			this->restore_stack_after_call();
		}

		asmjit::Error call(void* target);
		asmjit::Error jmp(void* target);
	};
	
	class detour
	{
	public:
		detour() = default;
		detour(void* place, void* target);
		detour(uintptr_t place, void* target);
		~detour();

		detour(detour&& other) noexcept
		{
			this->operator=(std::move(other));
		}

		detour& operator=(detour&& other) noexcept
		{
			if (this != &other)
			{
				this->~detour();

				this->place = other.place;
				this->original = other.original;
				this->moved_data = other.moved_data;

				other.place = nullptr;
				other.original = nullptr;
				other.moved_data = {};
			}

			return *this;
		}

		detour(const detour&) = delete;
		detour& operator=(const detour&) = delete;

		void enable();
		void disable();

		void create(void* place, void* target);
		void create(uintptr_t place, void* target);
		void clear();
		void move();

		static void disable_all();
		static void enable_all();

		void* get_original()
		{
			return this->original;
		}

		template <typename T>
		T* get() const
		{
			return static_cast<T*>(this->original);
		}

		template <typename T = void, typename... Args>
		T call(Args ... args)
		{
			return static_cast<T(*)(Args ...)>(this->original)(args...);
		}
	private:
		std::vector<uint8_t> moved_data{};
		void* place{};
		void* original{};

		void un_move();
	};

	template<typename T = void*>
	class vmt final
	{
	public:
		vmt() = delete;

		vmt(size_t num_entries) :
			instance_{ nullptr },
			num_entries_{ num_entries },
			table_new_{ new uintptr_t[num_entries] }
		{
			setup(instance_);
		}

		vmt(T instance, size_t num_entries) :
			instance_{ reinterpret_cast<uintptr_t**>(instance) },
			num_entries_{ num_entries },
			table_new_{ new uintptr_t[num_entries] }
		{
			setup(instance_);
		}

		~vmt()
		{
			setup(nullptr);

			delete[] table_new_;
		}

		void setup(T instance = nullptr)
		{
			if (instance_ && vptr_original_)
			{
				*instance_ = vptr_original_;
			}

			instance_ = reinterpret_cast<uintptr_t**>(instance);
			vptr_original_ = nullptr;

			const auto size{ num_entries_ * sizeof(std::remove_pointer<decltype(instance_)>::type) };

			if (instance_)
			{
				std::memcpy(table_new_, *instance_, size);
			}
			else
			{
				std::memset(table_new_, 0, size);
			}
		}

		template<typename F> void hook(size_t index, F function)
		{
			table_new_[index] = reinterpret_cast<uintptr_t>(function);

			if (!vptr_original_ && instance_)
			{
				vptr_original_ = *instance_;
				*instance_ = table_new_;
			}
		}

		template<typename T = void, typename... Args> auto call(const size_t index, Args... args)
		{
			auto orig_table = vptr_original_;

			if (!orig_table)
			{
				orig_table = *instance_;
			}

			return reinterpret_cast<T(*)(Args...)>(orig_table[index])(args...);
		}

	private:
		uintptr_t* table_new_;
		uintptr_t* vptr_original_{ nullptr };
		uintptr_t** instance_{ nullptr };
		size_t num_entries_{ 0 };
	};

	void* get_memory_near(const uintptr_t address, const size_t size);
	uint8_t* allocate_somewhere_near(const uintptr_t base_address, const size_t size);
	void retn(const uintptr_t address);
	void nop(const uintptr_t address, const size_t size);
	void nop(const void* place, const size_t size);
	std::vector<uint8_t> move_hook(const uintptr_t address);
	std::vector<uint8_t> move_hook(const void* pointer);
	void* assemble(const std::function<void(assembler&)>& asm_function);

	template <typename T> void copy(const uintptr_t address, const T data, const size_t length)
	{
		DWORD old_protect{ 0 };
		VirtualProtect(reinterpret_cast<void*>(address), length, PAGE_EXECUTE_READWRITE, &old_protect);

		std::memmove(reinterpret_cast<void*>(address), data, length);

		VirtualProtect(reinterpret_cast<void*>(address), length, old_protect, &old_protect);

		FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(address), length);
	}
	
	template <typename T> void copy(const void* place, const T data, const size_t length)
	{
		copy<T>(uintptr_t(place), data, length);
	}
	
	template <typename T> void set(const uintptr_t address, const T value)
	{
		DWORD old_protect{ 0 };
		VirtualProtect(reinterpret_cast<void*>(address), sizeof value, PAGE_EXECUTE_READWRITE, &old_protect);

		*reinterpret_cast<T*>(address) = value;

		VirtualProtect(reinterpret_cast<void*>(address), sizeof value, old_protect, &old_protect);

		FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(address), sizeof value);
	}

	template <typename T> void set(const void* place, const T value)
	{
		set<T>(uintptr_t(place), value);
	}

	template <typename T> bool is_relatively_far(const uintptr_t address, const T function, const int offset =  5)
	{
		const int64_t diff = uintptr_t(function) - address - offset;
		const auto small_diff = int32_t(diff);
		return diff != int64_t(small_diff);
	}

	template <typename T> void jump(const uintptr_t address, const T function, const bool use_far = false, const bool use_safe = false)
	{
		if (!use_far && is_relatively_far(address, function))
		{
			auto* trampoline = get_memory_near(address, 14);
			if (!trampoline)
			{
				throw std::runtime_error("Too far away to create 32bit relative branch");
			}
			
			jump(address, trampoline, false, false);
			jump(trampoline, function, true, true);
			return;
		}
		
		if (use_far)
		{
			const static uint8_t jump_data[] =
			{
				0x48, 0xB8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xFF, 0xE0
			}; 

			const static uint8_t jump_data_safe[] =
			{
				0xFF, 0x25, 0x00, 0x00, 0x00, 0x00
			};

			const auto data = use_safe ? jump_data_safe : jump_data;
			const auto jump_size = use_safe ? sizeof(jump_data_safe) : 2;

			copy(address, data, sizeof(data));
			copy(address + jump_size, &function, sizeof(function));
		}
		else
		{
			set(address, instr::jmp);
			set<uint32_t>(address + 1, uintptr_t(function) - address - 5);
		}
	}

	template <typename T> void jump(const void* place, const T function, const bool use_far = false, const bool use_safe = false)
	{
		jump<T>(uintptr_t(place), function, use_far, use_safe);
	}

	template <typename T> void call(const uintptr_t address, const T function)
	{
		if (is_relatively_far(address, function))
		{
			auto* trampoline = get_memory_near(address, 14);
			if (!trampoline)
			{
				throw std::runtime_error("Too far away to create 32bit relative branch");
			}

			call(address, trampoline);
			jump(trampoline, function, true, true);
			return;
		}
		
		set(address, instr::call);
		set<uint32_t>(address + 1, uintptr_t(function) - address - 5);
	}

	template <typename T> void return_value(const uintptr_t address, const T value)
	{
		set(address, instr::mov);
		set(address + 1, uint32_t(value));
		set(address + 5, instr::ret);
	}

	template <typename T> auto iat(const std::string& mod_name, const std::string& proc_name, T function)
	{
		const nt::library main{};
		if (!main.is_valid()) return T();

		auto ptr = main.get_iat_entry(mod_name, proc_name);
		if (!ptr) return T();

		auto original = *ptr;
		set(ptr, uintptr_t(function));
		return reinterpret_cast<T>(original);
	}

	template <typename T = void*> auto extract(void* place)
	{
		const auto data = static_cast<uint8_t*>(place);
		const auto offset = *reinterpret_cast<int*>(data);
		
		return reinterpret_cast<T>(data + offset + 4);
	}
}