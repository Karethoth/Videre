#include "typedefs.hh"


namespace
{
	size_t get_unicode_octet_count( const char byte )
	{
		if( (byte & 0b10000000) == 0 )
		{
			return 1;
		}
		else if( (byte & 0b11100000) == 0b11000000 )
		{
			return 2;
		}
		else if( (byte & 0b11110000) == 0b11100000 )
		{
			return 3;
		}
		else if( (byte & 0b11111000) == 0b11110000 )
		{
			return 4;
		}
		else return 1;
	}



	unsigned long u8_char_to_unicode( unsigned long c )
	{
		unsigned long val = 0;

		if( (c & 0xf8000000) == 0xf0000000 )
		{
			val |= (c & 0x7000000) >> 6;
			val |= (c & 0x3f0000) >> 4;
			val |= (c & 0x3f00) >> 2;
			val |= (c & 0x3f);
		}
		else if( (c & 0xf00000) == 0xe00000 )
		{
			val |= (c & 0xf0000) >> 4;
			val |= (c & 0x3f00) >> 2;
			val |= (c & 0x3f);
		}
		else if( (c & 0xe000) == 0xc000 )
		{
			val |= (c & 0x1f00) >> 2;
			val |= (c & 0x3f);
		}
		else val = c;

		return val;
	}


	string_unicode::value_type read_next_unicode_code_point( const char *str, const char * const end, size_t &bytes )
	{
		unsigned long character = 0;

		if( str >= end )
		{
			bytes = 1;
			return 0;
		}

		auto octet_count = get_unicode_octet_count( *str );
		if( (str + octet_count) > end )
		{
			octet_count = end - str;
		}

		bytes = octet_count ? octet_count : 1;

		character |= str[0] & 0xff;
		str++;
		while( octet_count-- > 1 )
		{
			character <<= 8;
			character |= str++[0] & 0xff;
		}

		return u8_char_to_unicode( character );
	}
}



string_unicode u8_to_unicode( const string_u8 &str )
{
	string_unicode unicode_str;
	unicode_str.reserve( str.size() );

	const char* str_ptr = str.data();
	const char* const str_end = str.data() + str.size();
	string_unicode::value_type code_point;
	size_t bytes_read = 0;

	while( (code_point = read_next_unicode_code_point( str_ptr, str_end, bytes_read )) && bytes_read )
	{
		unicode_str.push_back( code_point );
		str_ptr += bytes_read;
	}

	unicode_str.shrink_to_fit();
	return unicode_str;
}


