//Taken from - https://www.nfollmer.com/2017/01/09/bitmap-noise-generator-c-code/
//Used to generate images from perlin noise

#ifndef CREATEIMAGE_H
#define CREATEIMAGE_H

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include "perlin.h"

using namespace std;

#define FHEADERSIZE 14

typedef struct{
    unsigned short type;
    unsigned int fSize;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;
}BMPFileHeader;

typedef struct{
    unsigned int hSize;
    int iWidth;
    int iHeight;
    unsigned short iPlanes;
    unsigned short iBPP; //Bits per pixel
    unsigned int iCompression;
    unsigned int iSize;
    int i_xPPM; //Pixel per Meter
    int i_yPPM;
    unsigned int iClrs;
    unsigned int iClrsImp;
}BMPInfoHeader;

typedef struct{
    unsigned char b;
    unsigned char g;
    unsigned char r;
}RGBColor; //24 Bit Color Struct

int createImage(int height, int width,  std::vector<float> noiseMap, std::string filename);

#endif