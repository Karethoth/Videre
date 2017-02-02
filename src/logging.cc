#include "logging.hh"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <string>
#include <ctime>

using namespace logging;


namespace
{
	string_u8 filepath_to_filename( const string_u8 &filepath )
	{
		const char delim_unix[] = { '/' };
		const char delim_win[] = { '\\' };
		const auto end_unix = std::find_end( filepath.cbegin(), filepath.cend(), delim_unix, delim_unix+1 );
		const auto end_win = std::find_end( filepath.cbegin(), filepath.cend(), delim_win, delim_win+1 );
		const auto end = (end_unix != filepath.cend() && end_unix > end_win) ? end_unix : end_win;

		if( end != filepath.cend() )
		{
			return filepath.substr( end - filepath.cbegin() + 1 );
		}
		else return filepath;
	}

	string_u8 timestamp()
	{
		std::time_t t = std::time( nullptr );
		struct tm tmp;

#ifdef _WIN32
		localtime_s( &tmp, &t );
		const auto stamp = std::put_time( &tmp, "%Y-%m-%d-%H:%M:%S%z" );
#else
		const auto stamp = std::put_time( localtime( &t ), "%Y-%m-%d-%H:%M:%S%z" );
#endif

		std::stringstream iss;
		iss << stamp;
		return iss.str();
	}
};


// Static variables
decltype(Logger::log_files) Logger::log_files;
decltype(Logger::log_files_mutex) Logger::log_files_mutex;


LogFile::LogFile( std::string _path ) : path{ _path }, stream{ _path } {}
LogFile::LogFile( LogFile &_other ) : path{_other.path}, stream{_other.path} { }
LogFile::LogFile( LogFile &&_other ) : path{std::move(_other.path)}, stream{std::move(_other.stream)} { }


void LogFile::write( const string_u8 &text ) const
{
	std::lock_guard<std::mutex> lock{ write_mutex };
	stream.write( text.c_str(), text.size() );
}


void Logger::add_log_file( const LogCategory category, std::string path )
{
	std::lock_guard<std::mutex> log_files_lock{ Logger::log_files_mutex };
	auto &tmp = Logger::log_files[category];
	tmp.emplace_back( path );
}


void Logger::log( const LogCategory category, const string_u8 text, const string_u8 filepath = {}, const int line = 0 )
{
	std::lock_guard<std::mutex> log_files_lock{ Logger::log_files_mutex };
	const auto& files = Logger::log_files[category];

	const auto filename = filepath_to_filename( filepath );
	const auto stamp = timestamp();

	for( const auto& file : files )
	{
		file.write( stamp + ' ' + filename + ':' + std::to_string(line) + " - " + text + '\n' );
	}
}
