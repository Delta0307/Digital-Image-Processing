#pragma once
#include<stdint.h>
#include<cstdio>
#define VNAME(value) (#value)

#define FILE_ERROR "fail to open file!\n"
#define PARAMETER_ERROR "parameter error!\n"
#define COLORTABLE_ERROR "no color table info!\n"


typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

static BYTE byteLimited(int32_t p);

template<typename T>
void sort(T* vector, int32_t size);

template<typename T>
void divide(T* vector, int32_t head, int32_t rear);



static BYTE byteLimited(int32_t p)
{
	return p < 0 ? 0 : (p > 255 ? 255 : p);
};

template<typename T>
void sort(T* vector, int32_t size)
{
	if (size < 2)return;
	divide(vector, 0, size - 1);
}

template<typename T>
void divide(T* vector, int32_t head, int32_t rear)
{
	if (head >= rear)return;
	int32_t i = head, j = rear;
	T target = vector[head];
	while (i < j)
	{
		while (i < j && vector[j] >= target)--j;
		vector[i] = vector[j];
		while (i < j && vector[i] < target)++i;
		vector[j] = vector[i];
	}
	vector[i] = target;
	divide(vector, head, i - 1);
	divide(vector, i + 1, rear);
}

//3 Bytes
typedef struct PIXEL
{
    BYTE    blue;
    BYTE    green;
    BYTE    red;
    PIXEL() :blue(0), green(0), red(0) {}
    PIXEL(BYTE b, BYTE g, BYTE r) :blue(b), green(g), red(r) {}
    PIXEL(BYTE g) :blue(g), green(g), red(g) {}
    PIXEL operator=(const PIXEL& b)
    {
        blue = b.blue;
        green = b.green;
        red = b.red;
        return *this;
    }
    bool operator!=(const PIXEL& b)
    {
        return blue != b.blue || green != b.green || red != b.red;
    }
    bool operator==(const PIXEL& b)
    {
        return !(*this!=b);
    }
    PIXEL operator+(const PIXEL& b)
    {
        PIXEL a;
        a.blue = byteLimited((int32_t)this->blue + (int32_t)b.blue);
        a.green = byteLimited((int32_t)this->green + (int32_t)b.green);
        a.red = byteLimited((int32_t)this->red + (int32_t)b.red);
        return a;
    }
    PIXEL operator-(const PIXEL& b)
    {
        PIXEL a;
        a.blue = byteLimited((int32_t)this->blue - (int32_t)b.blue);
        a.green = byteLimited((int32_t)this->green - (int32_t)b.green);
        a.red = byteLimited((int32_t)this->red - (int32_t)b.red);
        return a;
    }
    PIXEL operator*(const int32_t& k)
    {
        PIXEL a;
        a.blue = byteLimited(k * (int32_t)this->blue);
        a.green = byteLimited(k * (int32_t)this->green);
        a.red = byteLimited(k * (int32_t)this->red);
        return a;
    }
    PIXEL operator*(const PIXEL& b)
    {
        PIXEL a;
        a.blue = byteLimited((int32_t)this->blue * (int32_t)b.blue);
        a.green = byteLimited((int32_t)this->green * (int32_t)b.green);
        a.red = byteLimited((int32_t)this->red * (int32_t)b.red);
        return a;
    }
    PIXEL operator/(const PIXEL& b)
    {
        PIXEL a;
        a.blue = byteLimited((int32_t)this->blue / (int32_t)b.blue);
        a.green = byteLimited((int32_t)this->green / (int32_t)b.green);
        a.red = byteLimited((int32_t)this->red / (int32_t)b.red);
        return a;
    }
    PIXEL operator^(const PIXEL& b)
    {
        PIXEL a;
        a.blue = byteLimited((int32_t)this->blue ^ (int32_t)b.blue);
        a.green = byteLimited((int32_t)this->green ^ (int32_t)b.green);
        a.red = byteLimited((int32_t)this->red ^ (int32_t)b.red);
        return a;
    }
    PIXEL operator+=(const PIXEL& b)
    {
        return *this + b;
    }
    PIXEL operator-=(const PIXEL& b)
    {
        return *this - b;
    }
    PIXEL operator*=(const PIXEL& b)
    {
        return *this * b;
    }
    PIXEL operator/=(const PIXEL& b)
    {
        return *this / b;
    }
    PIXEL operator^=(const PIXEL& b)
    {
        return *this ^ b;
    }
    int32_t operator,(const PIXEL& b)
    {
        
        return sqrt((b.blue - blue) * (b.blue - blue) +
            (b.green - green) * (b.green - green) +
            (b.red - red) * (b.red - red)
        );
        
        //return b.blue + b.green + b.red - blue - green - red;
    }
} PIXEL;