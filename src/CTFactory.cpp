/**
 * @file CTFactory.cpp
 * 
 * CTFactory implementation file
 */
#include "CTFactory.h"

#include <iostream>
#include <thread>


/**
 * Attempts to load an image from a file path
 * 
 * @param im pointer to a CImg object to load the image into
 * @param p the image file path
 */
static CImg<unsigned char>* initImage(const fs::path& p) {
    CImg<unsigned char>* im = nullptr;
    try {
        im = new CImg<unsigned char>(p.c_str());
        return im;
    } catch(const CImgIOException&) {
        std::cerr << "Could not initialize image at " << p << std::endl;
        std::cerr << "The file either doesn't exist or the file format is invalid." << std::endl;
    }
    return nullptr;
}


/**
 * Creates a WGSettings object from given parameters
 * @param props the CTSettings object provided by a CTFactory object
 * @param width the width of the gradient
 * @param height the height of the gradient
 * @param isCorner if the desired gradient should be a corner piece, otherwise edge piece
 */
static WGSettings getWGS(const CTSettings& props, int width, int height, bool isCorner) {
    WGSettings wgs;
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
    if((topImage = initImage(topImagePath)) == nullptr) return;
    if((bottomImage = initImage(bottomImagePath)) == nullptr) return;
    inWidth = topImage->width();
    inHeight = topImage->height();
    channels = std::min(topImage->spectrum(), bottomImage->spectrum());
    outImage = new CImg<unsigned char>(topImage->width() * 5,
                                       topImage->height() * 3,
                                       topImage->depth(),
                                       channels);

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

    applyBaseTile(true, inWidth, inHeight);

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

    outImage->save(outImagePath.c_str());
}

CTFactory::~CTFactory() {
    delete topImage;
    delete bottomImage;
    delete outImage;
    topImage = nullptr;
    bottomImage = nullptr;
    outImage = nullptr;
}

void CTFactory::applyTileBlend(const WalkingGradient& g, int xOffset, int yOffset, bool inverse) {
    CImg<unsigned char>& t_im = inverse ? *bottomImage : *topImage;
    CImg<unsigned char>& b_im = inverse ? *topImage : *bottomImage;
    CImg<unsigned char>& o_im = *outImage;
    int yIn = 0, yOut = yOffset, xIn = 0, xOut = xOffset;
    for(; yIn < inHeight; ++yIn, ++yOut) {
        for(; xIn < inWidth; ++xIn, ++xOut) {
            float r = g.getValue(xIn, yIn);
            float ir = 1.0 - r;
            for(int c = 0; c < channels; ++c) {
                o_im(xOut, yOut, 0, c) = (unsigned char)(ir * (float)t_im(xIn, yIn, 0, c) + r * (float)b_im(xIn, yIn, 0, c));
            }
        }
        xIn = 0;
        xOut = xOffset;
    }
}

void CTFactory::applyBaseTile(bool useTopImage, int xOffset, int yOffset) {
    CImg<unsigned char>& t_im = useTopImage ? *topImage : *bottomImage;
    CImg<unsigned char>& o_im = *outImage;
    int yIn = 0, yOut = yOffset, xIn = 0, xOut = xOffset;
    for(; yIn < inHeight; ++yIn, ++yOut) {
        for(; xIn < inWidth; ++xIn, ++xOut) {
            for(int c = 0; c < channels; ++c) {
                o_im(xOut, yOut, 0, c) = t_im(xIn, yIn, 0, c);
            }
        }
        xIn = 0;
        xOut = xOffset;
    }
}

void CTFactory::generateNETile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, true);
    WalkingGradient wg(wgs);
    wg.flipY();
    applyTileBlend(wg, 2 * inWidth, 0);
}

void CTFactory::generateNWTile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, true);
    WalkingGradient wg(wgs);
    wg.flipX();
    wg.flipY();
    applyTileBlend(wg, 0, 0);
}

void CTFactory::generateSETile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, true);
    WalkingGradient wg(wgs);
    applyTileBlend(wg, 2 * inWidth, 2 * inHeight);
}

void CTFactory::generateSWTile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, true);
    WalkingGradient wg(wgs);
    wg.flipX();
    applyTileBlend(wg, 0, 2 * inHeight);
}

void CTFactory::generateNTile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, false);
    WalkingGradient wg(wgs);
    wg.flipY();
    applyTileBlend(wg, inWidth, 0);
}

void CTFactory::generateSTile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, false);
    WalkingGradient wg(wgs);
    applyTileBlend(wg, inWidth, 2 * inHeight);
}

void CTFactory::generateETile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, false);
    WalkingGradient wg(wgs);
    wg.transpose();
    applyTileBlend(wg, 2 * inWidth, inHeight);
}

void CTFactory::generateWTile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, false);
    WalkingGradient wg(wgs);
    wg.transpose();
    wg.flipX();
    applyTileBlend(wg, 0, inHeight);
}

void CTFactory::generateNEInverseTile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, true);
    WalkingGradient wg(wgs);
    wg.flipY();
    applyTileBlend(wg, 4 * inWidth, 0, true);
}

void CTFactory::generateNWInverseTile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, true);
    WalkingGradient wg(wgs);
    wg.flipX();
    wg.flipY();
    applyTileBlend(wg, 3 * inWidth, 0, true);
}

void CTFactory::generateSEInverseTile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, true);
    WalkingGradient wg(wgs);
    applyTileBlend(wg, 4 * inWidth, inHeight, true);
}

void CTFactory::generateSWInverseTile() {
    WGSettings wgs = getWGS(props, inWidth, inHeight, true);
    WalkingGradient wg(wgs);
    wg.flipX();
    applyTileBlend(wg, 3 * inWidth, inHeight, true);
}