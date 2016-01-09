#pragma once

#include <iostream>
#include <vector>
#include <type_traits>

namespace tools
{

// owner<T> pointer alias
template<typename T>
using owner = T;



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



std::wstring read_file_contents( const std::string& filename );


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

}; // namespace tools

