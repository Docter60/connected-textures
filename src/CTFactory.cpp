/**
 * @file CTFactory.cpp
 * 
 * CTFactory implementation file
 */
#include "CTFactory.h"

#include <iostream>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "WalkingGradient.h"


/**
 * Creates a WGSettings object from given parameters
 * @param props the CTSettings object provided by a CTFactory object
 * @param width the width of the gradient
 * @param height the height of the gradient
 * @param isCorner if the desired gradient should be a corner piece, otherwise edge piece
 */
static WGSettings getWGS(const CTSettings& props, int width, int height, bool isCorner) {
    WGSettings wgs = WGSettings();
    wgs.sampleCount = props.sampleCount;
    wgs.width = width;
    wgs.height = height;
    wgs.isCorner = isCorner;
    wgs.seamHeight = props.seamHeight;
    wgs.variance = props.variance;
    wgs.steepness = props.steepness;
    return wgs;
}

CTFactory::CTFactory(const fs::path& topImagePath, const fs::path& bottomImagePath, const fs::path& outImagePath, const CTSettings& props) : props(props) {
    
    topImage.pixels = stbi_load(topImagePath.string().c_str(), &topImage.x, &topImage.y, &topImage.c, 0);
    bottomImage.pixels = stbi_load(bottomImagePath.string().c_str(), &bottomImage.x, &bottomImage.y, &bottomImage.c, 0);

    // Check if top image could be loaded
    if (topImage.pixels == nullptr) {
        std::cerr << "Could not load top image" << std::endl;
        return;
    }

    // Check if bottom image could be loaded
    if (bottomImage.pixels == nullptr) {
        std::cerr << "Could not load bottom image" << std::endl;
        return;
    }

    // Check if images have same dimensions
    if (topImage.x != bottomImage.x || topImage.y != bottomImage.y) {
        std::cerr << "Top and bottom images do not share the same dimensions" << std::endl;
        return;
    }


    outImage.x = topImage.x * OUTPUT_TILE_WIDTH;
    outImage.y = topImage.y * OUTPUT_TILE_HEIGHT;
    outImage.c = (topImage.c < bottomImage.c) ? topImage.c : bottomImage.c;
    outImage.pixels = new unsigned char[outImage.x * outImage.y * outImage.c]();

    std::thread ne_thread(&CTFactory::generateNETile, this);
    std::thread nw_thread(&CTFactory::generateNWTile, this);
    std::thread se_thread(&CTFactory::generateSETile, this);
    std::thread sw_thread(&CTFactory::generateSWTile, this);
    std::thread n_thread(&CTFactory::generateNTile, this);
    std::thread s_thread(&CTFactory::generateSTile, this);
    std::thread e_thread(&CTFactory::generateETile, this);
    std::thread w_thread(&CTFactory::generateWTile, this);
    std::thread ne_inv_thread(&CTFactory::generateNEInverseTile, this);
    std::thread nw_inv_thread(&CTFactory::generateNWInverseTile, this);
    std::thread se_inv_thread(&CTFactory::generateSEInverseTile, this);
    std::thread sw_inv_thread(&CTFactory::generateSWInverseTile, this);

    applyBaseTile(true, topImage.x, topImage.y);

    ne_thread.join();
    nw_thread.join();
    se_thread.join();
    sw_thread.join();
    n_thread.join();
    s_thread.join();
    e_thread.join();
    w_thread.join();
    ne_inv_thread.join();
    nw_inv_thread.join();
    se_inv_thread.join();
    sw_inv_thread.join();

    stbi_write_png(outImagePath.string().c_str(), outImage.x, outImage.y, outImage.c, outImage.pixels, 0);
}

CTFactory::~CTFactory() {
    if (topImage.pixels != nullptr)
        stbi_image_free(topImage.pixels);
    topImage.pixels = nullptr;
    if (bottomImage.pixels != nullptr)
        stbi_image_free(bottomImage.pixels);
    bottomImage.pixels = nullptr;
    if (outImage.pixels != nullptr) {
        delete[] outImage.pixels;
        outImage.pixels = nullptr;
    }
}

void CTFactory::applyTileBlend(const WalkingGradient& g, int xOffset, int yOffset, bool inverse) {
    const ImageData& tData = !inverse ? topImage : bottomImage;
    const ImageData& bData = !inverse ? bottomImage : topImage;
    unsigned char* o_im = outImage.pixels;

    for (int y = 0; y < tData.y; ++y) {
        int yOut = yOffset + y;
        for (int x = 0; x < tData.x; ++x) {
            int xOut = xOffset + x;
            int outInd = (yOut * outImage.x + xOut) * outImage.c;
            int t_inInd = (y * tData.x + x) * tData.c;
            int b_inInd = (y * bData.x + x) * bData.c;
            float r = g.getValue(x, y);
            float ir = 1.f - r;
            for (int c = 0; c < outImage.c; ++c) {
                o_im[outInd + c] = (unsigned char)(ir * (float)tData.pixels[t_inInd + c] + r * (float)bData.pixels[b_inInd + c]);
            }
        }
    }
}

void CTFactory::applyBaseTile(bool useTopImage, int xOffset, int yOffset) {
    const ImageData& im = useTopImage ? topImage : bottomImage;
    unsigned char* t_im = im.pixels;
    unsigned char *o_im = outImage.pixels;
    
    for (int y = 0; y < im.y; ++y) {
        int yOut = yOffset + y;
        for (int x = 0; x < im.x; ++x) {
            int xOut = xOffset + x;
            int outInd = (yOut * outImage.x + xOut) * outImage.c;
            int inInd = (y * im.x + x) * im.c;
            for (int c = 0; c < outImage.c; ++c) {
                o_im[outInd + c] = t_im[inInd + c];
            }
        }
    }
}

void CTFactory::generateNETile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, true);
    WalkingGradient wg(wgs);
    wg.flipY();
    applyTileBlend(wg, 2 * topImage.x, 0);
}

void CTFactory::generateNWTile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, true);
    WalkingGradient wg(wgs);
    wg.flipX();
    wg.flipY();
    applyTileBlend(wg, 0, 0);
}

void CTFactory::generateSETile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, true);
    WalkingGradient wg(wgs);
    applyTileBlend(wg, 2 * topImage.x, 2 * topImage.y);
}

void CTFactory::generateSWTile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, true);
    WalkingGradient wg(wgs);
    wg.flipX();
    applyTileBlend(wg, 0, 2 * topImage.y);
}

void CTFactory::generateNTile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, false);
    WalkingGradient wg(wgs);
    wg.flipY();
    applyTileBlend(wg, topImage.x, 0);
}

void CTFactory::generateSTile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, false);
    WalkingGradient wg(wgs);
    applyTileBlend(wg, topImage.x, 2 * topImage.y);
}

void CTFactory::generateETile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, false);
    WalkingGradient wg(wgs);
    wg.transpose();
    applyTileBlend(wg, 2 * topImage.x, topImage.y);
}

void CTFactory::generateWTile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, false);
    WalkingGradient wg(wgs);
    wg.transpose();
    wg.flipX();
    applyTileBlend(wg, 0, topImage.y);
}

void CTFactory::generateNEInverseTile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, true);
    WalkingGradient wg(wgs);
    wg.flipY();
    applyTileBlend(wg, 4 * topImage.x, 0, true);
}

void CTFactory::generateNWInverseTile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, true);
    WalkingGradient wg(wgs);
    wg.flipX();
    wg.flipY();
    applyTileBlend(wg, 3 * topImage.x, 0, true);
}

void CTFactory::generateSEInverseTile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, true);
    WalkingGradient wg(wgs);
    applyTileBlend(wg, 4 * topImage.x, topImage.y, true);
}

void CTFactory::generateSWInverseTile() {
    WGSettings wgs = getWGS(props, topImage.x, topImage.y, true);
    WalkingGradient wg(wgs);
    wg.flipX();
    applyTileBlend(wg, 3 * topImage.x, topImage.y, true);
}