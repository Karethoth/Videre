#pragma once

#include "window.hh"

#include <mutex>
#include <vector>


struct Globals
{
	static bool                     should_quit;

	static std::mutex               windows_mutex;
	static std::vector<gui::Window> windows;
};

