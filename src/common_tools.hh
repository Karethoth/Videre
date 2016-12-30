#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <type_traits>

#include "logging.hh"

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



// Flatten - vector<vector<X>> -> vector<X>
template<
	typename VectorType,
	typename ValueType=VectorType::value_type::value_type
>
std::vector<ValueType> flatten( VectorType vec )
{
	std::vector<ValueType> flattened;

	for( auto& sub_vec : vec )
	{
		flattened.insert( flattened.end(), sub_vec.cbegin(), sub_vec.cend() );
	}

	return flattened;
}



// Map
template<
	typename VectorType,
	typename TargetType,
	typename SourceType=VectorType::value_type
>
std::vector<TargetType> map_vector(
	VectorType vec,
	std::function<TargetType( SourceType )> transform
)
{
	std::vector<TargetType> mapped;

	for( SourceType element : vec )
	{
		mapped.push_back( transform( element ) );
	}

	return mapped;
}



// Helpers for splitting
template<typename VectorType>
std::vector<VectorType> split(
	const VectorType &vec,
	std::function<size_t( const VectorType&, size_t )> find_split_index
)
{
	size_t offset = 0;
	std::vector<VectorType> parts;

	while( offset < vec.size() )
	{
		const auto index = find_split_index( vec, offset );

		parts.emplace_back(
			vec.cbegin() + offset,
			vec.cbegin() + index
		);

		offset = index;
	}

	return parts;
}


template<typename StringType, typename CharType=StringType::value_type>
std::vector<StringType> split_string(
	const StringType &str,
	const CharType separator
)
{
	std::istringstream iss{ str };

	std::vector<StringType> tokens {
		istream_iterator<StringType>{iss},
		istream_iterator<StringType>{}
	};

	return tokens;
}

template<typename StringType, typename CharType=StringType::value_type>
std::vector<StringType> split_strings(
	const std::vector<StringType> &vec,
	const CharType separator
)
{
	std::vector<StringType> all_tokens;

	for( auto& str : vec )
	{
		std::istringstream iss{ str };

		std::vector<StringType> tokens {
			istream_iterator<StringType>{iss},
			istream_iterator<StringType>{}
		};

		vec.insert( vec.end(), tokens.begin(), tokens.end() );
	}

	return all_tokens;
}



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
			auto asd = __FILE__;
			auto wasd = __LINE__;
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
	std::ifstream in( filename, std::ios_base::in | std::ios_base::binary );
	if( !in.is_open() )
	{
		throw std::runtime_error( std::string{ "Couldn't open the file: '" } + filename + "'" );
	}

	T str{
		std::istreambuf_iterator<char>( in ),
		std::istreambuf_iterator<char>()
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

