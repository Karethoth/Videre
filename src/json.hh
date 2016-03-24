#pragma once

#include <map>
#include <memory>
#include <vector>
#include "common_tools.hh"

namespace json
{
using std::map;
using std::vector;
using std::wstring;
using std::shared_ptr;

struct Element;


enum ValueType {
	NILL,
	STRING,
	NUMBER,
	OBJECT,
	ARRAY,
	BOOLEAN
};


struct Number
{
	wstring value;
};


struct Element
{
	wstring   key;
	ValueType type;

	wstring                     value_string;
	Number                      value_number;
	map<wstring, Element>       value_object;
	vector<shared_ptr<Element>> value_array;
	bool                        value_boolean;

	Element( const wstring &_key );
	std::wstring parse( const wstring& );

protected:
	std::wstring parse_string( const wstring& );
	std::wstring parse_number( const wstring& );
	std::wstring parse_object( const wstring& );
	std::wstring parse_array( const wstring& );
	std::wstring parse_boolean( const wstring& );
};



Element read_file( const std::string &path );
Element parse( const wstring &str );

} // namespace json

