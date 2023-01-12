#pragma once
#include "dependencies/std_include.hpp"

namespace utils::nt
{
	class library final
	{
	public:
		static library load(const std::string& name);
		static library load(const std::filesystem::path& path);
		static library get_by_address(const void* address);
		static library get_by_address(const uintptr_t address);

		library();
		explicit library(const std::string& name);
		explicit library(HMODULE handle);

		library(const library& a) : module_(a.module_)
		{
		}

		bool operator!=(const library& obj) const { return !(*this == obj); };
		bool operator==(const library& obj) const;

		operator bool() const;
		operator HMODULE() const;

		void unprotect() const;
		void* get_entry_point() const;
		size_t get_relative_entry_point() const;

		bool is_valid() const;
		std::string get_name() const;
		std::string get_path() const;
		std::string get_folder() const;
		std::uint8_t* get_ptr() const;
		void free();

		HMODULE get_handle() const;

		template <typename T>
		T get_proc(const std::string& process) const
		{
			if (!this->is_valid()) T{};
			return reinterpret_cast<T>(GetProcAddress(this->module_, process.data()));
		}

		template <typename T>
		std::function<T> get(const std::string& process) const
		{
			if (!this->is_valid()) return std::function<T>();
			return static_cast<T*>(this->get_proc<void*>(process));
		}

		template <typename T, typename... Args>
		T call(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__cdecl)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		std::vector<PIMAGE_SECTION_HEADER> get_section_headers() const;

		PIMAGE_NT_HEADERS get_nt_headers() const;
		PIMAGE_DOS_HEADER get_dos_header() const;
		PIMAGE_OPTIONAL_HEADER get_optional_header() const;

		uintptr_t** get_iat_entry(const std::string& module_name, const std::string& proc_name) const;
	private:
		HMODULE module_;
	};

	template <HANDLE InvalidHandle = nullptr>
	class handle
	{
	public:
		handle() = default;

		handle(const HANDLE h)
			: handle_(h)
		{
		}

		~handle()
		{
			if (*this)
			{
				CloseHandle(this->handle_);
				this->handle_ = InvalidHandle;
			}
		}

		handle(const handle&) = delete;
		handle& operator=(const handle&) = delete;

		handle(handle&& obj) noexcept
			: handle()
		{
			this->operator=(std::move(obj));
		}

		handle& operator=(handle&& obj) noexcept
		{
			if (this != &obj)
			{
				this->~handle();
				this->handle_ = obj.handle_;
				obj.handle_ = InvalidHandle;
			}

			return *this;
		}

		handle& operator=(HANDLE h) noexcept
		{
			this->~handle();
			this->handle_ = h;

			return *this;
		}

		operator bool() const
		{
			return this->handle_ != InvalidHandle;
		}

		operator HANDLE() const
		{
			return this->handle_;
		}

	private:
		HANDLE handle_{ InvalidHandle };
	};

	bool is_shutdown_in_progress();
	void terminate(const uint32_t code);
}