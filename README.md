# Learning Digital Image Processing，Third Edition

> Rafael C. Gonzalez
>
> Richard E. Woods

# Document

> In this project, all image format is 24bits BMP, in order to simplify the operation of loading. 
>
> Compile the project by CMake is the best. Call all functions in main().

CMakeLists.txt: 

```txt
cmake_minimum_required (VERSION 2.8)

project (LearningDIP)

aux_source_directory(. SRC_LIST)

add_executable(main ${SRC_LIST})
```

```include "bmp.h"```

## Load image

```c++
bmpMat(const char* path);
bmpMat(const bmpMat& mat);
```
## Describe a pixel

The objects of this class support operator overloading, including 4 basic operations.

```c++
PIXEL() :blue(0), green(0), red(0) {}
PIXEL(BYTE b, BYTE g, BYTE r) :blue(b), green(g), red(r) {}
PIXEL(BYTE g) :blue(g), green(g), red(g) {}
```
## Output image

```c++
static bool bmpMat::writeBmp(bmpMat* mat, const char* path);
```

## Gray level transformation&spatial filtering

These are member functions in ```class bmpMat```, which  need an instantiated object before they can be called.

```c++
   	//gray level transformation
    bool bmpMat::toGray();//Gray = (R * 19595 + G * 38469 + B * 7472) >> 16
    bool bmpMat::threshold(int32_t t);//0<=t<=255
    bool bmpMat::reversal();//s = L - 1 - r
    bool bmpMat::grayscaleCompression();//s = clog(1+r);c = 255/log(256)
    bool bmpMat::grayscaleExpansion();//s = exp(cr)-1;c = log(256)/255
    bool bmpMat::gamma(double gamma);//s = cr^gamma;c = 255^(1-r)
    bool bmpMat::contrastStretch(int32_t r, int32_t s);//0<=r<=127;0<=s<=127
    bool bmpMat::bitPlane(int32_t b);//0<=b<=7
    //operation on histogram
    bool bmpMat::updateHistogram();
    int32_t** bmpMat::getHistogram();
    void bmpMat::printHistogram();
    bool bmpMat::histigramEqualization();

    //spatial filter
    bool bmpMat::meanFilter(int32_t n);
    bool bmpMat::gaussFilter(int32_t n);
    bool bmpMat::medianFilter(int32_t n);

    bool bmpMat::laplaceFilter();
    bool bmpMat::laplaceSharpen();
    bool bmpMat::unsharpMask(int32_t n);//good for word image
    bool bmpMat::highFrequrncyEmphasis(int32_t n, int32_t k);
    bool bmpMat::sobelFilter();
    bool bmpMat::sharpen();

	//image fusion
    bmpMat* getMask(uint8_t threshold);
    bool getFuse(bmpMat* foreground, bmpMat* mask, int32_t x, int32_t y);
```



# My understanding of some key and important points in book

## Formula 3.3-3

>  page 74

$$
p_s(s)=p_r(r) \left|\frac{dr}{ds}\right|
$$

$p_s(s)$ and $p_r(r)$ respectively stands for the probability density function of $s$ and r. $s$ and r respectively stands for the gray level of after and before transformation. $s=T(r)$.

Assume $p_s(s)$ and $p_r(r)$ is continuous, as for the gray level s in $p_s(s)$, it is  obvious that:

$$
\int^{s}_0p_s(s)ds=\int^{r}_0p_r(r)dr
$$

Because $s=T(r)$ are injection and strictly monotonic functions. So $r=H(s)$ is the same. That is:

$$
\int^{s}_0p_s(s)ds=\int^{H(s)}_0p_r(r)dr
$$

and take the derivative of both side of this equation with respect to $s$. That is:

$$
p_s(s)=p_r(H(s))H'(s)=p_r(r) \left|\frac{dr}{ds}\right|
$$

## Correlation and convolution

Although I have never studied signals, in the field of linear filtering in image process, I think correlation describes a operation for each pixel that take the neighborhood of the pixel as input arguments, and respectively  multiply by each coefficient in the filtering, and sum them up, and figure out the new value of the pixel. 

Convolution is similar to correlation, but in the operation of convolution, we should rotate the coefficient matrix 180 degree at the beginning.

## Fourier transform

try to understand...