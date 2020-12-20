/*
dr_opt.cpp
*/

#include <iostream>
#include <unordered_map>

#include "dr_opt.h"

namespace dr {
	using ArgMap = std::unordered_map<std::string, std::string>;

	static ArgMap& getoptmap() {
		static ArgMap options;
		return options;
	}

	void setopt(int argc, char* args[]) {
		ArgMap& options = getoptmap();
		// Filter out options into map here
		std::string arg = "";
		for (int i = 1; i < argc; ++i) {
			arg = args[i];
			if (arg[0] == '-' && arg.length() >= 2) {
				// Found option. Check if string parameter is present.
				int j = i + 1;
				if (j < argc) {
					std::string argpar = args[j];
					if (argpar[0] != '-') {
						options.emplace(arg.erase(0, 1), argpar);
						++i;
					}
					else {
						options.emplace(arg.erase(0, 1), "");
					}
				}
				else {
					options.emplace(arg.erase(0, 1), "");
				}
			}
			else {
				std::cout << "Ignoring standalone arguments" << std::endl;
			}
		}
	}


	const std::string& getopt(const std::string& flag) {
		ArgMap& options = getoptmap();
		// Retrieve options here
		return options.at(flag);
	}


	bool hasopt(const std::string& flag) {
		ArgMap& options = getoptmap();
		// Check if option exists in command here
		return options.find(flag) != options.end();
	}


	int getoptc() {
		ArgMap& options = getoptmap();
		return options.size();
	}
}