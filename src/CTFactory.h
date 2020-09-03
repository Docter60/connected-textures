/**
 * @file CTFactory.h
 * 
 * CTFactory header file
 */
#pragma once

#include <filesystem>

namespace cimg_library {
    template<typename T>
    struct CImg;
}

class WalkingGradient;

namespace fs = std::filesystem;

using namespace cimg_library;

/**
 * Definition of the CTSettings struct
 * 
 * Settings used to generate a connected texture
 */
struct CTSettings {
    int sampleCount;
    float variance;
    float steepness;
    float seamHeight;
};


/**
 * Definition of the CTFactory class
 * 
 * description
 */
class CTFactory {
    public:

        /**
         * CTFactory constructor
         * 
         * @param topImagePath file path to top image
         * @param bottomImagePath file path to bottom image
         * @param outImagePath file path to output image
         * @param props connected texture generation settings
         */
        CTFactory(const fs::path& topImagePath, const fs::path& bottomImagePath, const fs::path& outImagePath, const CTSettings& props);


        /**
         * Default destructor
         * 
         * Cleans up pointer members
         */
        virtual ~CTFactory();
    
    private:
        CImg<unsigned char> *topImage = nullptr, *bottomImage = nullptr, *outImage = nullptr;
        CTSettings props;
        int inWidth, inHeight, channels;

        /**
         * Uses color blending to apply a connected texture to the output image
         * 
         * @param g a walking gradient determining how the textures should blend
         * @param xOffset the starting x position on the output image to draw to
         * @param yOffset the starting y position on the output image to draw to
         * @param inverse if true, will swap top and bottom image
         */
        void applyTileBlend(const WalkingGradient& g, int xOffset, int yOffset, bool inverse = false);


        /**
         * Copies one of the base tiles into the output image
         * 
         * @param topImage if true, use top image, otherwise use bottom image
         * @param xOffset the starting x position on the output image to draw to
         * @param yOffset the starting y position on the output image to draw to
         */
        void applyBaseTile(bool useTopImage, int xOffset, int yOffset);


        /**
         * Tile functions used for generating each connected tile texture
         * 
         * Entry points for threads
         */
        void generateNETile();
        void generateNWTile();
        void generateSETile();
        void generateSWTile();
        void generateNTile();
        void generateSTile();
        void generateETile();
        void generateWTile();
        void generateNEInverseTile();
        void generateNWInverseTile();
        void generateSEInverseTile();
        void generateSWInverseTile();
};