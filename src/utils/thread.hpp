#pragma once
#include "dependencies/std_include.hpp"

namespace utils::thread
{
	class handle
	{
	public:
		handle(const DWORD thread_id, const DWORD access = THREAD_ALL_ACCESS)
			: handle_(OpenThread(access, FALSE, thread_id))
		{
		}

		operator bool() const
		{
			return this->handle_;
		}

		operator HANDLE() const
		{
			return this->handle_;
		}

	private:
		nt::handle<> handle_{};
	};
	
	std::vector<DWORD> get_thread_ids();
}