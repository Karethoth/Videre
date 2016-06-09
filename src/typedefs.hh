#pragma once
#include <string>
#include <vector>

// To tell UTF-8 and unicode code point strings apart
using string_u8 = std::string;
using string_unicode = std::vector<uint32_t>;


// owner<T> pointer alias
template<typename T>
using owner = T;

