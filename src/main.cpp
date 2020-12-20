/**
 * @file main.cpp
 * 
 * Main file holding the entry main function for the connected textures program
 */
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include "dr_opt.h"
#include "CTFactory.h"

namespace fs = std::filesystem;


/**
 * Prints program usage information
 */
static void printUsage() {
    std::cout << "Usage:" << std::endl;
}


/**
 * Stores a default settings file in the settingsFilePath
 * 
 * @param settingsFilePath the path to store the settings file
 */
static void createDefaultSettingsFile(const fs::path& settingsFilePath) {
    std::ofstream outf;
    outf.open(settingsFilePath);
    outf << "The parser only looks for lines with equal signs, so comments can exist." << std::endl;
    outf << "Spaces are allowed, but no new line whitespace." << std::endl;
    outf << std::endl;
    outf << "The amount of samples the program will use in a walking gradient" << std::endl;
    outf << "sampleCount = 129" << std::endl;
    outf << std::endl;
    outf << "Where the walking gradient will start the halfway seam" << std::endl;
    outf << "seamHeight = 64" << std::endl;
    outf << std::endl;
    outf << "The reach of the gradient's blending in pixels using euclidean distance" << std::endl;
    outf << "steepness = 10" << std::endl;
    outf << std::endl;
    outf << "The amplification applied to the walking gradient algorithm" << std::endl;
    outf << "variance = 5" << std::endl;
    outf.close();
}

/**
 * Parses a settings file for properties and places the strings in an unordered map
 * 
 * @param settingsFilePath path to an existing settings file
 * @param settings the unordered map to store settings in
 */
static void parseSettingsFile(const fs::path& settingsFilePath, std::unordered_map<std::string, std::string>& settings) {
    std::ifstream inf;
    inf.open(settingsFilePath);
    std::string line;
    while(std::getline(inf, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
        int delimiterIndex = line.find('=');
        std::string key, value;
        if(line.find('=') != std::string::npos) {
            key = line.substr(0, delimiterIndex);
            value = line.substr(delimiterIndex + 1);
            settings[key] = value;
        }
    }
    inf.close();
}


/**
 * Main entry point of the program
 */
int main(int argc, char* args[]) {
    dr::setopt(argc, args);
    // Check for correct number of options/arguments
    int optCount = dr::getoptc();
    if(optCount < 3) {
        printUsage();
        return 0;
    }

    // Load in options/arguments
    bool tFlag = false, bFlag = false, oFlag = false;
    fs::path topImagePath;
    fs::path bottomImagePath;
    fs::path outImagePath;
    fs::path settingsPath;

    // Get top image path
    if (dr::hasopt("t")) {
        topImagePath = fs::path(dr::getopt("t"));
        tFlag = true;
    }

    // Get bottom image path
    if (dr::hasopt("b")) {
        bottomImagePath = fs::path(dr::getopt("b"));
        bFlag = true;
    }

    // Get output image path
    if (dr::hasopt("o")) {
        outImagePath = fs::path(dr::getopt("o"));
        if (!fs::is_directory(outImagePath.parent_path())) {
            std::cerr << "Path to " << outImagePath << " does not exist." << std::endl;
            return 0;
        }
        oFlag = true;
    }

    // Get settings path
    if (dr::hasopt("s")) {
        settingsPath = fs::path(dr::getopt("s"));
    }

    // If required options weren't loaded, exit program
    if(!tFlag || !bFlag || !oFlag) {
        std::cerr << "Program requires top, bottom, and output image location" << std::endl;
        printUsage();
        return 0;
    }

    // If optional settings file wasn't specified, look for default settings file
    if(settingsPath == fs::path()) {
        settingsPath = fs::current_path() / "settings.txt";
        // If default file doesn't exist, make one
        if(!fs::exists(settingsPath))
            createDefaultSettingsFile(settingsPath);
    } else {
        // If optional settings file was specified but doesn't exist, exit program
        if(!fs::exists(settingsPath)) {
            std::cerr << "Could not find settings file." << std::endl;
            return 0;
        }
    }

    // Load in settings from settings file
    std::unordered_map<std::string, std::string> settings;
    parseSettingsFile(settingsPath, settings);

    // Load settings into CTSettings object, default if no property found in file
    CTSettings cts = CTSettings();
    auto it = settings.find("sampleCount");
    if(it != settings.end()) {
        cts.sampleCount = std::stoi(it->second);
    } else {
        cts.sampleCount = 129;
    }
    it = settings.find("seamHeight");
    if(it != settings.end()) {
        cts.seamHeight = std::stof(it->second);
    } else {
        cts.seamHeight = 64.0f;
    }
    it = settings.find("steepness");
    if(it != settings.end()) {
        cts.steepness = std::stof(it->second);
    } else {
        cts.steepness = 10.0f;
    }
    it = settings.find("variance");
    if(it != settings.end()) {
        cts.variance = std::stof(it->second);
    } else {
        cts.variance = 5.0f;
    }
    
    // Start the connected textures factory
    CTFactory ctf(topImagePath, bottomImagePath, outImagePath, cts);
    return 0;
}