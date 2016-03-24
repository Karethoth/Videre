#include "settings.hh"
#include "globals.hh"

#include <thread>

using namespace std;

mutex settings::settings_mutex;
nlohmann::json settings::core;

struct Tm : std::tm
{
	Tm(
		const int year, const int month, const int mday,
		const int hour, const int min, const int sec
	)
	{
		tm_year = year - 1900;
		tm_mon  = month - 1;
		tm_mday = mday;
		tm_hour = hour;
		tm_min  = min;
		tm_sec  = sec;
	}

	auto to_time_point()
	{
		auto time_c = mktime( this );
		return chrono::system_clock::from_time_t( time_c );
	}
};


thread tools::run_when_file_updated(
	const string path,
	const function<void(void)> func,
	const atomic_bool &stop,
	const chrono::milliseconds check_interval
)
{
	return thread( [=, &stop]
	{
		auto last_modified = chrono::system_clock::time_point();

		while( !stop.load() )
		{
			auto next_run = chrono::steady_clock::now() + check_interval;
			try
			{

			auto modified = file_modified( path );
			if( modified > last_modified )
			{
				last_modified = modified;
				func();
			}
			}
			catch( runtime_error &e )
			{
				wcout << "Error: " << e.what() << endl;
			}

			this_thread::sleep_until( next_run );
		}

	} );
}



#ifdef _WIN32

#include <windows.h>

chrono::system_clock::time_point tools::file_modified( const std::string &path )
{
	HFILE handle = 0;
	OFSTRUCT file_info;

	handle = OpenFile( path.c_str(), &file_info, OF_READ );
	if( !handle )
	{
		throw runtime_error( "File doesn't exist" );
	}

	FILETIME write_time;
	auto success = GetFileTime( (HANDLE)handle, NULL, NULL, &write_time );
	_lclose( handle );

	if( !success )
	{
		throw runtime_error( "GetFileTime failed" );
	}

	SYSTEMTIME system_time;
	success = FileTimeToSystemTime( &write_time, &system_time );
	if( !success )
	{
		throw runtime_error( "FileTimeToSystemTime failed" );
	}

	Tm tm{
		system_time.wYear, system_time.wMonth, system_time.wDay,
		system_time.wHour, system_time.wMinute, system_time.wSecond
	};

	return tm.to_time_point();
}
#endif

