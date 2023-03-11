#include <iostream>
using namespace std;
#include <opencv2/opencv.hpp>
using namespace cv;


#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"



int main() {
    int width, height, channels;
    unsigned char *img = stbi_load("../in.jpg", &width, &height, &channels, 0);
    if(img == NULL) {
        cout << "Error loading image" << endl;
        exit(1);
    }

    cout << "Loaded image with width of " << width << ", height of " << height << ", and " << channels << " channels."  << endl;

    // STEP 1: convert to gray --> referenced from: https://www.youtube.com/watch?v=1OyQoPCp46o
    size_t img_size = width * height * channels;
    // get number of channels depending if input image has alpha channel or not
    int gray_channels = channels == 4 ? 2 : 1;
    size_t gray_img_size = width * height * gray_channels;

    //allocate memory
    uint8_t* gray_img = new uint8_t[width * height * gray_channels];
    if(gray_img == NULL) {
        cout << "Unable to allocate memory for the gray image." << endl;
        exit(1);
    }
    // for loop is transformation to gray
    // p pointer iterates over pixels of the input image and pg pointer iterates over pixels of the output image
    for(unsigned char *p = img, *pg = gray_img; p != img + img_size; p += channels, pg += gray_channels) {
        // *p is red channel, *p+1 is green channel, and *p+2 is blue channel
        *pg = (uint8_t)((*p + *(p + 1) + *(p + 2)) / 3.0);
        // if image has 4 channels, copy value of 4th channel to 2nd gray image channel
        if(channels == 4) {
            *(pg + 1) = *(p + 3);
        }
    }
    // write to disk
    stbi_write_jpg("gray.jpg",width, height, gray_channels, gray_img, 100);

    //STEP 2: convert to black/white

    // free memory
    stbi_image_free(img);
    free(gray_img);



    /*ifstream image;
    ofstream newimage;

    image.open("../in.ppm");
    newimage.open("out.ppm");

    //copy over header information
    string type = "", width = "", height = "", RGB = "";
    image >> type;
    image >> width;
    image >> height;
    image >> RGB;

    cout << type << width << height << RGB << endl; */
    return 0;
}
