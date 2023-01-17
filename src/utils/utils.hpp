#pragma once
#include "dependencies/stdafx.hpp"

namespace utils
{
	bool are_addresses_equal(const game::netadr_t& a, const game::netadr_t& b);
	std::string get_sender_string(const game::netadr_t& from);
	void print_log(const char* msg, ...);

	template <class F>
	class final_action
	{
	public:
		static_assert(!std::is_reference<F>::value && !std::is_const<F>::value &&
			!std::is_volatile<F>::value,
			"Final_action should store its callable by value");

		explicit final_action(F f) noexcept : f_(std::move(f))
		{
		}

		final_action(final_action&& other) noexcept
			: f_(std::move(other.f_)), invoke_(std::exchange(other.invoke_, false))
		{
		}

		final_action(const final_action&) = delete;
		final_action& operator=(const final_action&) = delete;
		final_action& operator=(final_action&&) = delete;

		~final_action() noexcept
		{
			if (invoke_) f_();
		}

		// Added by momo5502
		void cancel()
		{
			invoke_ = false;
		}

	private:
		F f_;
		bool invoke_{ true };
	};

	template <class F>
	final_action<typename std::remove_cv<typename std::remove_reference<F>::type>::type>
		finally(F&& f) noexcept
	{
		return final_action<typename std::remove_cv<typename std::remove_reference<F>::type>::type>(
			std::forward<F>(f));
	}
}

inline bool operator==(const game::netadr_t& a, const game::netadr_t& b)
{
	return utils::are_addresses_equal(a, b);
}

inline bool operator!=(const game::netadr_t& a, const game::netadr_t& b)
{
	return !(a == b); 
}

namespace std
{
	template <>
	struct equal_to<game::netadr_t>
	{
		using result_type = bool;

		bool operator()(const game::netadr_t& lhs, const game::netadr_t& rhs) const
		{
			return utils::are_addresses_equal(lhs, rhs);
		}
	}; 
	
	template <>
	struct hash<game::netadr_t>
	{
		size_t operator()(const game::netadr_t& x) const noexcept
		{
			const auto type_hash = hash<uint32_t>()(x.type);

			if (x.type != game::NA_IP && x.type != game::NA_RAWIP)
			{
				return type_hash;
			}

			return type_hash ^ hash<uint32_t>()(x.inaddr) ^ hash<uint16_t>()(x.port);
		}
	};

	template<typename T1, typename T2>
	struct hash<pair<T1, T2>>
	{
		size_t operator()(const pair<T1, T2>& x) const noexcept
		{
			return hash<T1>()(x.first) ^ hash<T2>()(x.second);
		}
	};
}