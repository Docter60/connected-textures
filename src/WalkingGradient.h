/**
 * @file WalkingGradient.h
 * 
 * WalkingGradient header file
 */
#pragma once


/**
 * Definition of the WGSettings struct
 * 
 * Settings used to generate a walking gradient
 */
struct WGSettings {
    int width, height, sampleCount, seamHeight;
    float variance, steepness;
    bool isCorner;
};


/**
 * Definition of the WalkingGradient class
 * 
 * description
 */
class WalkingGradient {
    public:

        /**
         * Constructs a walking gradient based on given settings
         * 
         * @param wgs walking gradient settings
         */
        WalkingGradient(const WGSettings& wgs);


        /**
         * Default destructor
         * 
         * Cleans up data member
         */
        virtual ~WalkingGradient();

        float getValue(int x, int y) const;


        /**
         * Replaces all mapped gradient values g to 1.0 - g
         * 
         * Conserves gradient value's position
         */
        void invert();


        /**
         * Flips gradient values around the X axis
         */
        void flipY();


        /**
         * Flips gradient values around the Y axis
         */
        void flipX();


        /**
         * Transposes gradient like a 2D matrix
         */
        void transpose();


        /**
         * Saves a grayscale image of the gradient to the cwd
         * 
         * File name: debug_walking_gradient.png
         */
        void debug() const;

    private:
        float *data;
        int width, height;
};