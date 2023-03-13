#include <iostream>
using namespace std;

#include <cstdint>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <omp.h>
#include <iomanip>



int main() {
    int width, height, channels;
    unsigned char *img = stbi_load("../in.jpg", &width, &height, &channels, 0);

    cout << "Loaded image with width of " << width << ", height of " << height << ", and " << channels << " channels."  << endl;

    //// STEP 1: convert to gray --> referenced from: https://www.youtube.com/watch?v=1OyQoPCp46o
    size_t img_size = width * height * channels;
    // get number of channels depending on whether input image has alpha channel or not
    int gray_channels = channels == 4 ? 2 : 1;
    size_t gray_img_size = width * height * gray_channels;

    //allocate memory
    uint8_t* gray_img = new uint8_t[width * height * gray_channels];
    // for loop is transformation to gray
    // p pointer iterates over pixels of the input image and pg pointer iterates over pixels of the output image
    double start_time = omp_get_wtime();

#pragma omp parallel
    {
        for (unsigned char *p = img, *pg = gray_img; p != img + img_size; p += channels, pg += gray_channels) {
            // *p is red channel, *p+1 is green channel, and *p+2 is blue channel
            *pg = (uint8_t) ((*p + *(p + 1) + *(p + 2)) / 3.0);
            // if image has 4 channels, copy value of 4th channel to 2nd gray image channel
            if (channels == 4) {
                *(pg + 1) = *(p + 3);
            }
        }
    }
    double run_time = omp_get_wtime() - start_time;
    cout << "Gray image made in " << setprecision(6) << run_time << " seconds" << endl;

    // write to disk
    stbi_write_jpg("gray.jpg",width, height, gray_channels, gray_img, 100);


    //// Step 2: use local Maxima and Minima, Source: https://dl.acm.org/doi/pdf/10.1145/1815330.1815351 [implementation is my own but formula is from this paper]

    //allocate memory
    uint8_t* MM_img = new uint8_t[width * height * gray_channels];

    // construct a window that moves over the image to dynamically calculate an average threshold
    const int window = 11, winMinusTwo = window - 2;
    float localThreshold = 0;
    // conceptually you might think of this as a matrix, however, since we don't actually perform any matrix operations and just want to store the local Min and Max, we can just use a vector
    std::vector<int> v = {};
    for (int i = 0; i < window*window; i++){
        v.push_back(0);
    }
    start_time = omp_get_wtime();
    // this loop iterates over all rows besides last #window - 2
    for(unsigned char *p = gray_img, *pg = MM_img; p != gray_img + gray_img_size - (width*winMinusTwo + window*window - 1); p ++ , pg ++) { //find correct end of for loop, start was -2*width+2
        int localMax = INT_MIN;
        int localMin = INT_MAX;

        // the pointers fill the vector v with pixel values of a 3x3 window
        // if we add the width to the pointer, we gain pixel values from #widths lines under the current *p
        for(int i = 0; i < window*window; i++){
            v[i] = *(p + (i % window) + (((int)i / window) * width));
        }
        //// dealing with high contrast pixels
        int windowAverage = 0, threshAverage = 0, counter = 0, neighThreshold = 7;
        float stdSum = 0, stdd = 0;
        for(int i = 0; i < v.size(); i++){
            windowAverage += v[i];
        }
        windowAverage = windowAverage / v.size();
        std::vector<int> w = {};
        // count how many pixels are over that average
        for(int i = 0; i < v.size(); i++){
            if(v[i] > windowAverage){
                counter++;
                w.push_back(v[i]);
            }
        }

        //// CONDITION 1: If counter > threshold, we compute Emean and Estd, otherwise we don't need those computations
        if(counter > neighThreshold){
            //// Emean --> Mean of all high contrast pixels
            for(int i = 0; i < w.size(); i++){
                threshAverage += w[i];
            }
            threshAverage = threshAverage / w.size();
            //// Estd --> Std of all high contrast pixels
            for(int i = 0; i < w.size(); i++){
                stdSum += (v[0] - threshAverage)*(v[0] - threshAverage);
            }
            stdSum /= 2;
            stdd = sqrt(stdSum);

            //// CONDITION 2: If current pixel <= Emean + Estd/2
            if(v[0] <= ((threshAverage + stdd)/2)){
                *pg = 0;
            } else {
                *pg = 255;
            }
        } else {
            for(int i = 0; i < v.size(); i++){
                if(v[i] > localMax){
                    localMax = v[i];
                }
                if(v[i] < localMin){
                    localMin = v[i];
                }
            }
            localThreshold = (localMax - localMin) / (localMax + localMin + 0.00000000001) * 255; // constant to avoid div by 0
            //cout << localThreshold << endl;
            if(*p < localThreshold){
                *pg = 0;
            } else{
                *pg = 255;
            }
        }
    }
    // leftover rows
    for(unsigned char *p = gray_img + gray_img_size - (width*winMinusTwo + window*window - 1), *pg = MM_img + gray_img_size - (width*9 + window*window - 1), j = 0; p < gray_img + gray_img_size; p ++ , pg ++, j ++){
        int h = (int)j / width;
        for(int i = 0; i < window*window; i++){
            v[i] = *(p + (i % window) + ((h*((int)i / window)) * width));
        }

        //// dealing with high contrast pixels
        int localMax = INT_MIN;
        int localMin = INT_MAX;
        int windowAverage = 0, threshAverage = 0, counter = 0, neighThreshold = 25;
        float stdSum = 0, stdd = 0;
        for(int i = 0; i < v.size(); i++){
            windowAverage += v[i];
        }
        windowAverage = windowAverage / v.size();
        std::vector<int> w = {};
        // count how many pixels are over that average
        for(int i = 0; i < v.size(); i++){
            if(v[i] > windowAverage){
                counter++;
                w.push_back(v[i]);
            }
        }
        //// CONDITION 1: If counter > threshold, we compute Emean and Estd, otherwise we don't need those computations
        if(counter > neighThreshold){
            //// Emean --> Mean of all high contrast pixels
            for(int i = 0; i < w.size(); i++){
                threshAverage += w[i];
            }
            threshAverage = threshAverage / w.size();
            //// Estd --> Std of all high contrast pixels
            for(int i = 0; i < w.size(); i++){
                stdSum += (v[0] - threshAverage)*(v[0] - threshAverage);
            }
            stdSum /= 2;
            stdd = sqrt(stdSum);

            //// CONDITION 2: If current pixel <= Emean + Estd/2
            if(v[0] <= ((threshAverage + stdd)/2)){
                *pg = 0;
            } else {
                *pg = 255;
            }
        } else {
            for(int i = 0; i < v.size(); i++){
                if(v[i] > localMax){
                    localMax = v[i];
                }
                if(v[i] < localMin){
                    localMin = v[i];
                }
            }
            localThreshold = (localMax - localMin) / (localMax + localMin + 0.00000000001) * 255; // constant to avoid div by 0
            //cout << localThreshold << endl;
            if(*p < localThreshold){
                *pg = 0;
            } else{
                *pg = 255;
            }
        }
    }
    // write to disk
    stbi_write_jpg("MinMax.jpg",width, height, gray_channels, MM_img, 100);

    // free memory
    stbi_image_free(img);
    free(gray_img);
    free(MM_img);

    run_time = omp_get_wtime() - start_time;
    cout << "Binarization executed in " << setprecision(6) << run_time << " seconds" << endl;

    return 0;
}
