#pragma once
#include <string>
#include <vector>

// To tell UTF-8 and unicode code point strings apart
using string_u8 = std::string;
using string_unicode = std::vector<uint32_t>;

string_unicode u8_to_unicode( const string_u8 &str );


// owner<T> pointer alias
template<typename T>
using owner = T;

