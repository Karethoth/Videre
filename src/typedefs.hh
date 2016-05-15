#pragma once
#include <string>

// To tell UTF-8 strings apart
using string_u8 = std::string;

// owner<T> pointer alias
template<typename T>
using owner = T;

