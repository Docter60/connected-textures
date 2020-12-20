/*
dr_opt.hpp
*/
#pragma once

#include <string>

namespace dr {
	void setopt(int argc, char* args[]);
	const std::string& getopt(const std::string& flag);
	bool hasopt(const std::string& flag);
	int getoptc();
}