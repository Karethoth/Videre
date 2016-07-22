#pragma once
#include "common_tools.hh"


#include <json.hpp>

#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <functional>


namespace tools
{
	std::thread run_when_file_updated(
		const std::string path,
		const std::function<void( void )> func,
		const std::atomic_bool &stop,
		const std::chrono::milliseconds check_interval =
			std::chrono::milliseconds{ 1000 }
	);

	std::chrono::system_clock::time_point file_modified( const std::string &path );

}



namespace settings
{
	extern std::mutex settings_mutex;
	extern nlohmann::json core;
}

