/**
 * @file WalkingGradient.cpp
 * 
 * WalkingGradient implementation file
 */
#include "WalkingGradient.h"

#include <chrono>
#include <glm/vec2.hpp>
#include <glm/gtx/closest_point.hpp>
//#include <glm/gtx/polar_coordinates.hpp> // no vec2 function
#include <iomanip>
#include <iostream>
#include <limits>
#include <cmath>
#include <random>

#define cimg_display 0
#define cimg_use_png 1
#include "CImg.h"

using namespace cimg_library;


/**
 * Converts a euclidean/cartesian coordinate to a polar coordinate
 * 
 * @param u a euclidean/cartesian coordinate
 * @return polar coordinate
 */
static glm::vec2 polar(const glm::vec2& u) {
    return {std::sqrt(u.x * u.x + u.y * u.y), std::atan2(u.y, u.x)};
}


/**
 * Converts a polar coordinate to a euclidean/cartesian coordinate
 * 
 * @param u a polar coordinate
 * @return euclidean/cartesian coordinate
 */
static glm::vec2 euclidean(const glm::vec2& u) {
    return {u.x * std::cos(u.y), u.x * std::sin(u.y)};
}

static glm::vec2* generateEdgeSamples(const WGSettings& wgs) {
    glm::vec2 *samples = new glm::vec2[wgs.sampleCount];

    unsigned seed;
    std::default_random_engine generator;
    std::normal_distribution<float> distribution(0.0f, 0.5f);

    float width_ratio = wgs.width / (float)(wgs.sampleCount - 1);
    do {
        seed = std::chrono::system_clock::now().time_since_epoch().count();
        generator.seed(seed);
        samples[0] = {0.0f, wgs.seamHeight};
        for(int i = 1; i < wgs.sampleCount; ++i) {
            float movement = distribution(generator) * wgs.variance;
            samples[i] = {i * width_ratio, samples[i - 1].y + movement};
        }
    } while (std::abs(samples[0].y - samples[wgs.sampleCount - 1].y) > 1.0f);

    return samples;
}

static glm::vec2* generateCornerSamples(const WGSettings& wgs) {
    int sampleCount = M_PI_4 * wgs.sampleCount; // Quarter circle ratio
    glm::vec2 *samples = new glm::vec2[sampleCount];

    unsigned seed;
    std::default_random_engine generator;
    std::normal_distribution<float> distribution(0.0f, 0.5f);

    float width_ratio = M_PI_2 / (float)(sampleCount - 1);
    do {
        seed = std::chrono::system_clock::now().time_since_epoch().count();
        generator.seed(seed);
        samples[0] = {wgs.seamHeight, 0.0f};
        for(int i = 1; i < sampleCount; ++i) {
            float movement = distribution(generator) * wgs.variance;
            samples[i] = {samples[i - 1].x + movement, i * width_ratio};
        }
    } while (std::abs(samples[0].x - samples[sampleCount - 1].x) > 1.0f);

    return samples;
}

static float generateEdgeGradientPixel(glm::vec2* samples, int x, int y, const WGSettings& wgs) {
    float dw = wgs.width * wgs.height;
    glm::vec2 v = {x, y}, currentPoint, closestPoint;
    for(int i = 1; i < wgs.sampleCount; ++i) {
        currentPoint = glm::closestPointOnLine(v, samples[i], samples[i - 1]);
        float d = glm::distance(v, currentPoint);
        if(d < dw) {
            dw = d;
            closestPoint = currentPoint;
        }
    }
    if(closestPoint.y < y)
        return std::min(1.0f, 0.5f + dw / (2.0f * wgs.steepness));
    else
        return 0.5f - std::min(0.5f, dw / (2.0f * wgs.steepness));
}

static float generateCornerGradientPixel(glm::vec2* samples, int x, int y, const WGSettings& wgs) {
    int sampleCount = M_PI_4 * wgs.sampleCount; // Quarter circle ratio
    float dw = wgs.width * wgs.height;
    glm::vec2 v = {x, y}, currentPoint, closestPoint;
    for(int i = 1; i < sampleCount; ++i) {
        currentPoint = glm::closestPointOnLine(v, samples[i - 1], samples[i]);
        float d = glm::distance(v, currentPoint);
        if(d < dw) {
            dw = d;
            closestPoint = currentPoint;
        }
    }
    glm::vec2 polarClosestPoint = polar(closestPoint);
    glm::vec2 polarPixelPoint = polar(v);
    if(polarClosestPoint.x < polarPixelPoint.x)
        return std::min(1.0f, 0.5f + dw / (2.0f * wgs.steepness));
    else
        return 0.5f - std::min(0.5f, dw / (2.0f * wgs.steepness));
}

static void generateEdgeGradient(float *data, const WGSettings& wgs) {
    glm::vec2 *samples = generateEdgeSamples(wgs);

    for(int y = 0; y < wgs.height; ++y) {
        int yOffset = y * wgs.width;
        for(int x = 0; x < wgs.width; ++x) {
            data[yOffset + x] = generateEdgeGradientPixel(samples, x, y, wgs);
        }
    }

    delete samples;
}

static void generateCornerGradient(float *data, const WGSettings& wgs) {
    int sampleCount = M_PI_4 * wgs.sampleCount; // Quarter circle ratio
    glm::vec2 *samples = generateCornerSamples(wgs);
    glm::vec2 *eSamples = new glm::vec2[sampleCount];

    for(int i = 0; i < sampleCount; ++i) {
        eSamples[i] = euclidean(samples[i]);
    }

    delete samples;
    samples = nullptr;

    for(int y = 0; y < wgs.height; ++y) {
        int yOffset = y * wgs.width;
        for(int x = 0; x < wgs.width; ++x) {
            data[yOffset + x] = generateCornerGradientPixel(eSamples, x, y, wgs);
        }
    }

    delete eSamples;
}

static void generateGradient(float *data, const WGSettings& wgs) {
    if(wgs.isCorner)
        generateCornerGradient(data, wgs);
    else
        generateEdgeGradient(data, wgs);
}

WalkingGradient::WalkingGradient(const WGSettings& wgs) : width(wgs.width), height(wgs.height) {
    data = new float[width * height];
    generateGradient(data, wgs);


}

WalkingGradient::~WalkingGradient() {
    delete data;
    data = nullptr;
}

float WalkingGradient::getValue(int x, int y) const {
    return data[y * height + x];
}

void WalkingGradient::invert() {
    for(int i = 0; i < width * height; ++i) {
        data[i] = 1.0f - data[i];
    }
}

void WalkingGradient::flipX() {
    float temp;
    int yOffset;
    for(int y = 0; y < height; ++y) {
        yOffset = y * width;
        for(int x = 0; x < width / 2; ++x) {
            temp = data[yOffset + x];
            data[yOffset + x] = data[yOffset + (width - 1 - x)];
            data[yOffset + (width - 1 - x)] = temp;
        }
    }
}

void WalkingGradient::flipY() {
    float temp;
    int yOffset;
    for(int y = 0; y < height / 2; ++y) {
        yOffset = y * width;
        for(int x = 0; x < width; ++x) {
            temp = data[yOffset + x];
            data[yOffset + x] = data[(height - 1 - y) * width + x];
            data[(height - 1 - y) * width + x] = temp;
        }
    }
}

void WalkingGradient::transpose() {
    float temp;
    int yOffset;
    for(int y = 0; y < height; ++y) {
        yOffset = y * width;
        for(int x = y + 1; x < width; ++x) {
            temp = data[yOffset + x];
            data[yOffset + x] = data[x * width + y];
            data[x * width + y] = temp;
        }
    }
}

void WalkingGradient::debug() const {
    CImg<unsigned char> test(width, height);
    for(int y = 0; y < height; ++y) {
        int yOffset = y * width;
        for(int x = 0; x < width; ++x) {
            test(x, y) = (unsigned char)(255.0 * data[yOffset + x]);
        }
    }
    test.save("debug_walking_gradient.png");
}