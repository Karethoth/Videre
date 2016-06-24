#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <type_traits>

namespace tools
{

// can_call compile time type check
struct can_call_test
{
	template<typename F>
	static decltype(std::declval<F>()(), std::true_type())
	f( int );

	template<typename F, typename... A>
	static std::false_type
	f( ... );
};

template<typename F, typename...A>
using can_call = decltype(can_call_test::f<F>( 0 ));



// Defer
// - When the Defer falls out of scope, it executes
//   the function that was given to the constructor
// - Moving breaks the original Defer
template<typename F>
struct Defer
{
	Defer( F func )
	: is_broken(false),
	  call_when_destroyed(func)
	{
	}


	Defer( Defer&& old )
	: is_broken(false),
	  call_when_destroyed(old.call_when_destroyed)
	{
		old.is_broken = true;
	}


	~Defer() noexcept
	{
		static_assert( can_call<F>{}, "Defer<F> - F has to be a callable type!" );

		if( is_broken )
		{
			return;
		}

		try
		{
			call_when_destroyed();
		}
		catch( ... )
		{
			std::cout << "Defer<F> - Ran into an exception when called the deferred func!" << std::endl;
		}
	}


	// Delete potentially dangerous constructors and operators
	Defer( Defer& )             = delete;
	Defer& operator=( Defer& )  = delete;
	Defer& operator=( Defer&& ) = delete;


private:
	const F call_when_destroyed;
	bool is_broken;
};



// Helper for making defers
template <typename F>
constexpr auto make_defer( F func )
{
	return Defer<decltype(func)>{ func };
}



// int_to_float and float_to_int conversions
// - Because static_cast causes a warning from CppCoreCheck
//   and not casting causes a warning from compiler
// - Just two warnings here either way is better
inline float int_to_float( const int i ) noexcept
{
	return static_cast<float>( i );
}

inline int float_to_int( const float f ) noexcept
{
	return static_cast<int>( f );
}



// File I/O

template <typename T=std::wstring>
T read_file_contents( const std::string& filename )
{
	ifstream in( filename, ios_base::in | ios_base::binary );
	if( !in.is_open() )
	{
		throw runtime_error( std::string{ "Couldn't open the file: '" } + filename + "'" );
	}

	T str{
		istreambuf_iterator<char>( in ),
		istreambuf_iterator<char>()
	};

	return str;
}


// Directory listing
enum DirectoryItemType
{
	DIRECTORY,
	FILE
};

struct DirectoryItem
{
	DirectoryItemType type;
	std::string       name;
	size_t            size_h;
	size_t            size_l;
};



std::vector<DirectoryItem> get_directory_listing( std::string path );

bool is_file_readable( std::string path );

// String helpers

namespace str
{
	std::wstring ltrim( std::wstring s );
	std::wstring rtrim( std::wstring s );
	std::wstring trim( std::wstring s );
}; // namespace tools::str

}; // namespace tools

