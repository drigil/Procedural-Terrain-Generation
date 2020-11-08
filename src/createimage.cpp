#include "createimage.h"


int createImage(int height, int width, std::vector<float> noiseMap, std::string filename){ //Check if height and width required

    ofstream bmpfile;
    bmpfile.open(filename + ".bmp", ios::out | ios::binary | ios::trunc);
    if(!bmpfile.is_open())
    {
        cout << "ERROR: FILE COULD NOT BE OPENED" << endl;
        return 1;
    }
    
    // int width = 1024;
    // int height = 1024;
    
    BMPFileHeader fHeader;   //File Header 
    fHeader.type = 19778;    //BM
    fHeader.fSize = FHEADERSIZE + sizeof(BMPInfoHeader) + sizeof(RGBColor)*width*height;   //File Size info
    fHeader.fSize = sizeof(fHeader) + sizeof(RGBColor);
    fHeader.reserved1 = 0;
    fHeader.reserved2 = 0;
    fHeader.offset = FHEADERSIZE + sizeof(BMPInfoHeader);    //Where we actually start writing the Bitmap data/image
    
    bmpfile.write((char*)(&fHeader.type), sizeof(fHeader.type));     //Let's start writing the file
    bmpfile.write((char*)(&fHeader.fSize), sizeof(fHeader.fSize));   //Header information
    bmpfile.write((char*)(&fHeader.reserved1), sizeof(fHeader.reserved1));
    bmpfile.write((char*)(&fHeader.reserved2), sizeof(fHeader.reserved2));
    bmpfile.write((char*)(&fHeader.offset), sizeof(fHeader.offset));
    
    BMPInfoHeader iHeader;
    iHeader.hSize = sizeof(BMPInfoHeader);
    iHeader.iWidth = width;
    iHeader.iHeight = height;
    iHeader.iPlanes = 1;
    iHeader.iBPP = 24;
    iHeader.iCompression = 0; //Uncompressed
    iHeader.iSize = width * height * 3;
    iHeader.i_xPPM = 0;
    iHeader.i_yPPM = 0;
    iHeader.iClrs = 0;
    iHeader.iClrsImp = 0;
    
    bmpfile.write((char*)&iHeader, sizeof(BMPInfoHeader));   //More Header information being placed in the file
    
    RGBColor* image;
    image = new RGBColor[width * height];
    //Write each R G B value (this will produce color noise - for B&W, make each RGB value the same for every iteration
    for(int i = 0; i < width * height; i++) 
    {
        image[i].r = lerp(noiseMap[i], 0, 255);
        image[i].g = lerp(noiseMap[i], 0, 255);
        image[i].b = lerp(noiseMap[i], 0, 255);
    }   

    //Write our arrays to the file
    for(int i = 0; i < width * height; i++)
    {
        bmpfile.write((char*)&image[i], sizeof(RGBColor));
    }
    // remove our temp image array
    
    delete [] image;
   // close the file
    
    bmpfile.close();
    return 0;
    
}
