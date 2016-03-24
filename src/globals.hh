#pragma once

#include "window.hh"
#include "shaderProgram.hh"

#include <map>
#include <mutex>
#include <vector>
#include <atomic>


struct Globals
{
	static std::atomic_bool         should_quit;

	static std::mutex               windows_mutex;
	static std::vector<gui::Window> windows;

	static std::map<std::string, ShaderProgram> shaders;
};

