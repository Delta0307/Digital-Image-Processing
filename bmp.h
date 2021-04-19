#pragma once

//#include <Windows.h>
//#include <windef.h>
#include <cstdio>
#include <stdlib.h>
#include <math.h>
#include "def.h"
#include "mat.h"

#include<iostream>


//14 Bytes but occupy 16 Bytes
typedef struct tagBITMAPFILEHEADER
{
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER;

//40 Bytes
typedef struct tagBITMAPINFOHEADER
{
    DWORD      biSize;
    int32_t       biWidth;
    int32_t       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    int32_t       biXPelsPerMeter;
    int32_t       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
} BITMAPINFOHEADER;

//4 Bytes
typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;

class bmpMat
{
public:
    bmpMat(const char* path);
    bmpMat(const bmpMat& mat);
    bmpMat(const int32_t w, const int32_t h);

    ~bmpMat();

    static bool writeBmp(bmpMat* mat, const char* path);

    enum class color
    {
        BLUE,
        GREEN,
        RED
    };

    inline int32_t getIndex(color c, int32_t i, int32_t j);



    //gray level transformation
    bool toGray();//Gray = (R * 19595 + G * 38469 + B * 7472) >> 16
    bool threshold(int32_t t);//0<=t<=255
    bool reversal();//s = L - 1 - r
    bool grayscaleCompression();//s = clog(1+r);c = 255/log(256)
    bool grayscaleExpansion();//s = exp(cr)-1;c = log(256)/255
    bool gamma(double gamma);//s = cr^gamma;c = 255^(1-r)
    bool contrastStretch(int32_t r, int32_t s);//0<=r<=127;0<=s<=127
    bool bitPlane(int32_t b);//0<=b<=7
    //operation on histogram
    bool updateHistogram();
    int32_t** getHistogram();
    void printHistogram();
    bool histigramEqualization();

    //spatial filter
    bool meanFilter(int32_t n);
    bool gaussFilter(int32_t n);
    bool medianFilter(int32_t n);

    bool laplaceFilter();
    bool laplaceSharpen();
    bool unsharpMask(int32_t n);//good for word image
    bool highFrequrncyEmphasis(int32_t n, int32_t k);
    bool sobelFilter();
    bool sharpen();

    //cliping
    /***************************************************************
    *  x ^
    *    |
    *    |
    *    |
    *  0 |---------->y
    ***************************************************************/
    PIXEL moore();
    bmpMat* getROI(int32_t x, int32_t y, int32_t h, int32_t w);
    bmpMat* getNotROI(int32_t x, int32_t y, int32_t h, int32_t w);
    bmpMat* getMask(uint8_t threshold);
    bool getFuse(bmpMat* foreground, bmpMat* mask, int32_t x, int32_t y);
    bool getFuse(bmpMat* foreground, bmpMat* mask);
private:
    bmpMat();
    BITMAPFILEHEADER bmpFileHeader;
    BITMAPINFOHEADER bmpInfoHeader;


    int32_t lineByte;
    int32_t width;
    int32_t height;
    int64_t size;

    PIXEL** pixels;

    bool isGray;
    int32_t** histogram;

    //not yet complete
    bool scaleAdjust(int32_t h, int32_t w);
    bool adjustHeight(int32_t h);
    bool adjustWidth(int32_t w);
};