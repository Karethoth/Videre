#include "json.hh"

using namespace std;


wstring read_string( const wstring &data )
{
	if( data.size() < 2 )
	{
		throw runtime_error( "Data too short to contain a string" );
	}

	if( data[0] != '\"' )
	{
		throw runtime_error( "String is supposed to begin with a \"-character" );
	}

	bool escape = false;
	size_t str_length = 0;
	bool found_end = false;
	for( wchar_t c : data.substr( 1 ) )
	{
		if( c == '\\' )
		{
			escape = !escape;
		}

		else if( c == '\"' && !escape )
		{
			found_end = true;
			break;
		}

		if( escape ) escape = false;

		str_length++;
	}

	if( !found_end )
	{
		throw runtime_error( "Unexpected EOF: String doesn't end" );
	}

	return data.substr( 1, str_length );
}


json::Element::Element( const wstring &_key )
{
	key = _key;
}


wstring json::Element::parse( const wstring &str )
{
	auto trimmed = tools::str::trim( str );
	wcout << "Constructing from: \"" << trimmed.c_str() << "\"" << endl;

	auto test = wstring{ L"\"\"x testing how this turns out this is just some random excess stuff" };
	auto test_str = read_string( test );
	auto excess = test.substr( test_str.length()+2 );

	switch( trimmed[0] )
	{
		case '\"':
			return parse_string( trimmed );

		case '{':
			return parse_object( trimmed );

		case '[':
			return parse_array( trimmed );

		default:
			wcout << "Number, boolean or error: Not implemented" << endl;
	}

	return trimmed;
}


wstring json::Element::parse_string( const wstring &str )
{
	type = STRING;
	value_string = read_string( str );
	// TODO: parse escaped characters
	return str.substr( value_string.size() + 2 );
}


wstring json::Element::parse_object( const wstring &str )
{
	type = OBJECT;
	return str;
}


wstring json::Element::parse_array( const wstring &str )
{
	type = ARRAY;
	return str;
}


json::Element json::read_file( const string &path )
{
	wstring contents = tools::read_file_contents( path );
	return parse( contents );
}


json::Element json::parse( const wstring &str )
{
	json::Element root{ L"root" };
	root.parse( str );
	return root;
}

