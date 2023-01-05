#include "dependencies/std_include.hpp"
#include "utils.hpp"

namespace utils 
{
	bool are_addresses_equal(const game::netadr_t& a, const game::netadr_t& b)
	{
		if (a.type != b.type)
		{
			return false;
		}

		if (a.type != game::NA_RAWIP && a.type != game::NA_IP)
		{
			return true;
		}

		return a.port == b.port && a.inaddr == b.inaddr;
	}
	
	std::string get_sender_string(const game::netadr_t& from)
	{
		game::XNADDR xn;
		game::dwNetadrToCommonAddr(from, &xn, sizeof xn, nullptr);

		const auto ip_str{ xn.to_string() };
		const auto session = game::session_data();

		if (const auto client_num = game::find_target_from_addr(session, from); client_num >= 0)
		{
			const auto client = session->clients[client_num].activeClient;
			return utils::string::va("'%s' (%llu) %s", client->fixedClientInfo.gamertag, client->fixedClientInfo.xuid, ip_str.data());
		}
		else
		{
			return ip_str;
		}
	}
	
	void print_log(const char* msg, ...)
	{
		va_list ap;
		va_start(ap, msg);
		const auto result = utils::string::format(ap, msg);
		va_end(ap);

		const static auto file = utils::string::get_log_file("logs");
		utils::io::write_file(file, "[" + utils::string::data_time(0, false) + "] " + result, true);
	}
}