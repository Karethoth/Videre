#pragma once

#include "common_types.hh"

#include <mutex>
#include <vector>
#include <fstream>


namespace logging
{

	enum LogCategory
	{
		GENERAL,
		ERRORS
	};


	class LogFile
	{
		mutable std::ofstream stream;
		mutable std::mutex write_mutex;
		std::string path;

	  public:
		LogFile( std::string _path );
		LogFile( LogFile& );
		LogFile( LogFile&& );

		void write( const string_u8 &text ) const;
	};


	class Logger
	{
		using LogFileVector = std::vector<LogFile>;
		static std::map<LogCategory, LogFileVector> log_files;
		static std::mutex log_files_mutex;

	  public:
		static void add_log_file( const LogCategory category, std::string path );
		static void log( const LogCategory category, const string_u8 text, const string_u8 filepath, const int line );
	};

	#define LOG(CATEGORY, TEXT) logging::Logger::log(logging::LogCategory::CATEGORY, TEXT, __FILE__, __LINE__)
};


