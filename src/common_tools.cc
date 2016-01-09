#include "common_tools.hh"

#include <fstream>

using namespace std;
using namespace tools;



/* PLATFORM INDEPENDENT */
wstring tools::read_file_contents( const string& filename )
{
	ifstream in( filename, ios_base::in | ios_base::binary );
	if( !in.is_open() )
	{
		throw runtime_error( "Couldn't open the file!" );
	}

	wstring str{ istreambuf_iterator<char>( in ),
		istreambuf_iterator<char>() };

	return str;
}



/* WINDOWS-SPECIFIC */
#ifdef _WIN32
#include <windows.h>


vector<DirectoryItem> tools::get_directory_listing( string path )
{
	path += "*";

	HANDLE                handle;
	WIN32_FIND_DATAA      data;
	vector<DirectoryItem> items;

	handle = FindFirstFileA( path.c_str(), &data );

	if( handle == INVALID_HANDLE_VALUE )
	{
		throw runtime_error( "Couldn't get directory listing for '" + path + "'" );
	}

	auto defer_close_handle = make_defer( [&]()	{
		FindClose( handle );
	} );

	do
	{
		if( data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			items.push_back( {
				DirectoryItemType::DIRECTORY,
				data.cFileName,
				0,
				0
			} );
		}
		else
		{
			items.push_back( {
				DirectoryItemType::FILE,
				data.cFileName,
				data.nFileSizeHigh,
				data.nFileSizeLow
			} );
		}
	}
	while( FindNextFile( handle, &data ) );

	auto err = GetLastError();
	if( err != ERROR_NO_MORE_FILES )
	{
		throw runtime_error( "Error iterating over directory items" );
	}

	return items;
}

/* NON-WINDOWS-SPECIFIC */
#else

#include <dirent.h>
#include <sys/types.h>

vector<DirectoryItem> tools::get_directory_listing( string path )
{
  vector<DirectoryItem> items;

  DIR *handle;
  dirent *data;
  handle = opendir( path.c_str() );
  if( handle == nullptr )
  {
    throw runtime_error( "Couldn't get directory listing for '" + path + "'" );
  }

  while( (data = readdir( handle )) != nullptr )
  {
    auto type = (data->d_type == DT_DIR ? DirectoryItemType::DIRECTORY : DirectoryItemType::FILE);
    
    items.push_back({
        type,
        data->d_name,
        0, // Filesize is just ignored, atm
        0
    });
  }

  return items;
}

#endif

