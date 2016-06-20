#include "../src/common_tools.hh"

// Using https://github.com/philsquared/Catch
#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <iostream>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <windows.h>

#pragma comment( lib, "SDL2.lib" )
#pragma comment( lib, "SDL2_ttf.lib" )
#pragma comment( lib, "SDL2_image.lib" )
#endif

TEST_CASE( "Test test" )
{
	REQUIRE( 1 == 1 );

	SECTION( "true == true" )
	{
		REQUIRE( true == true );
	}
}


// Custom main provided to add the "press enter to quit" prompt on windows
int main( int argc, char * const argv[] )
{
	#ifdef _WIN32
	auto defer_enter_to_quit = tools::make_defer( []()
	{
		std::wcout << "\nPress enter to quit... ";
		std::cin.ignore();
	} );
	#endif

	Catch::Session session;
	int return_code = session.applyCommandLine( argc, argv );
	if( return_code != 0 )
	{
		return return_code;
	}

	return session.run();
}

