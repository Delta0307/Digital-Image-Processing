#include "bmp.h"

bmpMat::bmpMat(const char* path) :
	lineByte(0),
	width(0),
	height(0),
	size(0),
	isGray(false),
	histogram(new int32_t* [3]{ new int32_t[256](),new int32_t[256](),new int32_t[256]() })
{
	FILE* fp;
	if ((fp = fopen(path,"rb")) == NULL)
	{
		printf(FILE_ERROR);
		exit(0);
	}
	fread(&bmpFileHeader, 14, 1, fp);
	fread(&bmpInfoHeader, 40, 1, fp);
	//the number of Byte per row must be a multiple of 4
	lineByte = (bmpInfoHeader.biWidth * 3 + 3) / 4 * 4;
	height = bmpInfoHeader.biHeight;
	width = bmpInfoHeader.biWidth;
	size = (int64_t)lineByte * (int64_t)height;
	BYTE* buffer = new BYTE[size];
	fread(buffer, 1, size, fp);
	pixels = new PIXEL * [height];
	int32_t index = 0;
	for (int32_t i = 0;i < height;++i)
	{
		pixels[i] = new PIXEL[width];
		for (int32_t j = 0;j < width;++j)
		{
			pixels[i][j] = PIXEL(
				buffer[getIndex(color::BLUE, i, j)],
				buffer[getIndex(color::GREEN, i, j)],
				buffer[getIndex(color::RED, i, j)]
			);
		}
	}
	delete[] buffer;
	updateHistogram();
	/*
	printf("%u\n%llx\n%d\n%d\n%u\n\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
		bmpFileHeader.bfType, bmpFileHeader.bfSize, bmpFileHeader.bfReserved1, bmpFileHeader.bfReserved2,
		bmpFileHeader.bfOffBits, bmpInfoHeader.biSize, bmpInfoHeader.biWidth, bmpInfoHeader.biHeight,
		bmpInfoHeader.biPlanes, bmpInfoHeader.biBitCount, bmpInfoHeader.biCompression, bmpInfoHeader.biSizeImage,
		bmpInfoHeader.biXPelsPerMeter, bmpInfoHeader.biYPelsPerMeter, bmpInfoHeader.biClrUsed, bmpInfoHeader.biClrImportant);
	*/
	fclose(fp);
}

bmpMat::bmpMat(const bmpMat& mat):
	bmpFileHeader(mat.bmpFileHeader),
	bmpInfoHeader(mat.bmpInfoHeader),
	height(mat.height),
	width(mat.width),
	lineByte(mat.lineByte),
	size(mat.size),
	isGray(mat.isGray)
{
	pixels = new PIXEL * [height];
	for (int32_t i = 0;i < height;++i)
	{
		pixels[i] = new PIXEL[width];
		for (int32_t j = 0;j < width;++j)
		{
			pixels[i][j] = mat.pixels[i][j];
		}
	}
	histogram = new int32_t * [3];
	for (int32_t i = 0;i < 3;++i)
	{
		histogram[i] = new int32_t[256];
		for (int32_t j = 0;j < 256;++j)
		{
			histogram[i][j] = mat.histogram[i][j];
		}
	}
}

bmpMat::bmpMat(const int32_t w, const int32_t h):
	width(w),
	height(h),
	lineByte((w * 3 + 3) / 4 * 4),
	isGray(0),
	histogram(new int32_t* [3]{ new int32_t[256](),new int32_t[256](),new int32_t[256]() })
{
	size = (int64_t)lineByte * (int64_t)h;
	bmpFileHeader.bfType = 0x4D42;
	bmpFileHeader.bfSize = 54 + size;
	bmpFileHeader.bfReserved1 = 0;
	bmpFileHeader.bfReserved2 = 0;
	bmpFileHeader.bfOffBits = 3452764160;
	
	bmpInfoHeader.biSize = 40;
	bmpInfoHeader.biWidth = w;
	bmpInfoHeader.biHeight = h;
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 24;
	bmpInfoHeader.biCompression = 0;
	bmpInfoHeader.biSizeImage = size;
	bmpInfoHeader.biXPelsPerMeter = 0;
	bmpInfoHeader.biYPelsPerMeter = 0;
	bmpInfoHeader.biClrUsed = 0;
	bmpInfoHeader.biClrImportant = 0;

	pixels = new PIXEL*[height];
	for (int32_t i = 0;i < height;++i)
	{
		pixels[i] = new PIXEL[width]();
	}
	updateHistogram();
}

bmpMat::~bmpMat()
{
	for (int32_t i = 0;i < height;++i)
	{
		delete[] pixels[i];
	}
	delete[] pixels;
	for (int32_t i = 0;i < 3;++i)
	{
		delete[] histogram[i];
	}
	delete[] histogram;
}

bool bmpMat::writeBmp(bmpMat* mat, const char* path)
{
	if (mat == NULL)return 0;
	FILE* fp;
	if ((fp = fopen(path, "wb")) == NULL)
	{
		printf(FILE_ERROR);
		return 0;
	}
	fwrite(&mat->bmpFileHeader, 14, 1, fp);
	fwrite(&mat->bmpInfoHeader, 40, 1, fp);
	int32_t delta = mat->lineByte - 3 * mat->width;
	for (int32_t i = 0;i < mat->height;++i)
	{
		for (int32_t j = 0;j < mat->width;++j)
		{
			fwrite(&mat->pixels[i][j], 3, 1, fp);
		}
		if (delta)
		{
			BYTE reserved = 0;
			fwrite(&reserved, 1, delta, fp);
		}
	}
	fclose(fp);
	return 1;
}

inline int32_t bmpMat::getIndex(color c, int32_t i, int32_t j)
{
	return i * lineByte + 3 * j + (int32_t)c;
}

bool bmpMat::scaleAdjust(int32_t h, int32_t w)
{
	bmpMat* gradient = new bmpMat(*this);
	gradient->toGray();
	gradient->sobelFilter();

	return false;
}

bool bmpMat::adjustHeight(int32_t h)
{
	if (h == height)return 1;
	bmpMat* gradient = new bmpMat(*this);
	gradient->toGray();
	gradient->sobelFilter();
	int32_t delta = h - height;
	int32_t** dp = new int32_t * [height];
	for (int32_t i = 0;i < height;++i)
	{
		dp[i] = new int32_t[width];
		dp[i][0] = gradient->pixels[i][0].blue;
	}
	if (delta < 0)
	{
		delta = -delta;
		for (int32_t j = 1;j < width;++j)
		{
			for (int32_t i = 0;i < height;++i)
			{
				int32_t a = dp[i][j - 1], b = i - 1 < 0 ? 0 : dp[i - 1][j - 1], c = i + 1 >= height ? 0 : dp[i + 1][j - 1];
				int32_t min = a > b ? b : a;
				min = min > c ? c : min;
				dp[i][j] = min + gradient->pixels[i][j].blue;
			}
		}
		int32_t** path = new int32_t * [height];
		for (int32_t i = 0;i < height;++i)
		{
			path[i] = new int32_t[width];
			for (int32_t j = 0;j < width;++j)
			{
				path[i][j] = 0;
			}
		}
		for (int32_t j = width - 1;j > 0;--j)
		{
			for (int32_t i = 0;i < height;++i)
			{
				int32_t last = dp[i][j] - gradient->pixels[i][j].blue;
				if (last == dp[i][j - 1])path[i][j] = 0;
				else if (i > 0 && last == dp[i - 1][j - 1] || i <= 0 && last == 0)path[i][j] = -1;
				else path[i][j] = 1;
			}
		}
		for (int32_t n = 0;n < delta;++n)
		{
			int32_t index = 0, val = dp[0][width - 1];
			for (int32_t i = 1;i < height;++i)
			{
				if (dp[i][width - 1] < val)
				{
					index = i;
					val = dp[i][width - 1];
				}
			}
			printf("%d", index);
			for (int32_t j = width - 1, i = index;j >= 0;--j)
			{
				if (i<0 || i>height)break;
				pixels[i][j] = PIXEL(0);
				if (j > 0)i += path[i][j];
			}
			dp[index][width - 1] = INT32_MAX;
		}
		for (int32_t i = 0;i < height;++i)
		{
			delete[] path[i];
		}
		delete[] path;
	}
	else
	{
		return 0;
	}
	for (int32_t i = 0;i < height;++i)
	{
		delete[] dp[i];
	}
	delete[] dp;
	delete gradient;
	return 1;
}

bool bmpMat::adjustWidth(int32_t w)
{
	return false;
}

/******************************************************************************************************
grey level transformation
*******************************************************************************************************/

bool bmpMat::toGray()
{
	//Gray = (R * 19595 + G * 38469 + B * 7472) >> 16
	if (isGray)return 1;
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			int32_t gray = (pixels[i][j].blue * 7472 + pixels[i][j].green * 38469 + pixels[i][j].red * 19595) >> 16;
			pixels[i][j] = PIXEL(gray);
		}
	}
	updateHistogram();
	isGray = true;
	return 1;
}

bool bmpMat::threshold(int32_t t)
{
	if (t < 0 || t>255)return 0;
	if (!isGray)toGray();
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			uint8_t gray = pixels[i][j].blue >= t ? 255 : 0;
			pixels[i][j] = PIXEL(gray);
		}
	}
	updateHistogram();
	return 1;
}

bool bmpMat::reversal()
{
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			pixels[i][j] = PIXEL(255) - pixels[i][j];
		}
	}
	updateHistogram();
	return 1;
}

bool bmpMat::grayscaleCompression()
{
	//s = clog(1+r);c = 255/log(256)
	double c = 255 / log(256);
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			pixels[i][j] = PIXEL(
				c * log(1 + (double)pixels[i][j].blue),
				c * log(1 + (double)pixels[i][j].green),
				c * log(1 + (double)pixels[i][j].red)
			);
		}
	}
	updateHistogram();
	return 1;
}

bool bmpMat::grayscaleExpansion()
{
	//s = exp(cr)-1;c = log(256)/255
	double c = log(256) / 255;
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			pixels[i][j] = PIXEL(
				exp(c * (double)pixels[i][j].blue) - 1,
				exp(c * (double)pixels[i][j].green) - 1,
				exp(c * (double)pixels[i][j].red) - 1
			);
		}
	}
	updateHistogram();
	return 1;
}

bool bmpMat::gamma(double gamma)
{
	//s = cr^gamma;c = 255^(1-gamma)
	if (gamma <= 0)return 0;
	double c = pow(255, 1 - gamma);
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			pixels[i][j] = PIXEL(
				c * pow(pixels[i][j].blue, gamma),
				c * pow(pixels[i][j].green, gamma),
				c * pow(pixels[i][j].red, gamma)
			);
		}
	}
	updateHistogram();
	return 1;
}

bool bmpMat::contrastStretch(int32_t r, int32_t s)
{
	//0<=r<=127;0<=s<=127;r'=255-r;s'=255-s;
	if (r < 0 || r>127 || s < 0 || s>127)return 0;
	double r_ = 255.0 - (double)r,
		s_ = 255.0 - (double)s,
		delta_r = r_ - (double)r,
		delta_s = s_ - (double)r;
	//L: S = R*(s_-s)/255 + s
	if (r == 0)
	{
		for (int32_t i = 0;i < height;++i)
		{
			for (int32_t j = 0;j < width;++j)
			{
				pixels[i][j] = PIXEL(
					(double)pixels[i][j].blue * delta_s / 255 + (double)s,
					(double)pixels[i][j].green * delta_s / 255 + (double)s,
					(double)pixels[i][j].red * delta_s / 255 + (double)s
				);
			}
		}
	}
	//L: S = R*255/(r_-r) - r*255/(r_-r)
	else if (s == 0)
	{
		auto f = [&](uint8_t p)->uint8_t {
			if (p < r)return 0;
			else if (p < r_)return (double)p * 255 / delta_r - (double)r * 255 / delta_r;
			else return 255;
		};
		for (int32_t i = 0;i < height;++i)
		{
			for (int32_t j = 0;j < width;++j)
			{
				pixels[i][j] = PIXEL(
					f(pixels[i][j].blue),
					f(pixels[i][j].green),
					f(pixels[i][j].red)
				);
			}
		}
	}
	//L1: S = (s/r)R; L2: S = (s/r)R + 255(1-(s/r)); L3: S = (s'-s)/(r'-r)R + s - (s'-s)/(r'-r)r
	else
	{
		auto f = [&](uint8_t p)->uint8_t {
			if (p < r)return ((double)s / (double)r) * (double)p + 255 * (1 - ((double)s / (double)r));
			else if (p < r_)return ((double)s / (double)r) * (double)p + 255 * (1 - ((double)s / (double)r));
			else return (double)p * delta_s / delta_r + (double)s - (double)r * delta_s / delta_r;
		};
		for (int32_t i = 0;i < height;++i)
		{
			for (int32_t j = 0;j < width;++j)
			{
				pixels[i][j] = PIXEL(
					f(pixels[i][j].blue),
					f(pixels[i][j].green),
					f(pixels[i][j].red)
				);
			}
		}
	}
	updateHistogram();
	return 1;
}

bool bmpMat::bitPlane(int32_t b)
{
	if (b < 0 || b>7)return 0;
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			pixels[i][j] = PIXEL(
				((pixels[i][j].blue >> b) & 1) ? 255 : 0,
				((pixels[i][j].green >> b) & 1) ? 255 : 0,
				((pixels[i][j].red >> b) & 1) ? 255 : 0
			);
		}
	}
	updateHistogram();
	return 1;
}

bool bmpMat::updateHistogram()
{
	for (int32_t i = 0;i < 3;++i)
	{
		for (int32_t j = 0;j < 256;++j)
		{
			histogram[i][j] = 0;
		}
	}
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			++histogram[0][pixels[i][j].blue];
			++histogram[1][pixels[i][j].green];
			++histogram[2][pixels[i][j].red];
		}
	}
	return 1;
}

int32_t** bmpMat::getHistogram()
{
	return histogram;
}

void bmpMat::printHistogram()
{
	auto print = [&](int32_t* gray) {
		for (int32_t i = 0;i < 256;++i)
		{
			if (i < 10)printf("00%d:", (int)i);
			else if (i < 100)printf("0%d:", (int)i);
			else printf("%d:", (int)i);
			int32_t n =  gray[i]/10;
			for (int32_t j = 0;j < n;++j)
			{
				printf("*");
			}
			printf(" %f\n", (double)gray[i] / ((double)width * (double)height));
		}
	};
	if (isGray)
	{
		print(histogram[0]);
	}
	else
	{
		for (int32_t i = 0;i < 3;++i)
		{
			switch (i)
			{
			case 0:printf("\nblue:\n");break;
			case 1:printf("\ngreen:\n");break;
			case 2:printf("\nred\n");break;
			}
			print(histogram[i]);
		}
	}
}

bool bmpMat::histigramEqualization()
{
	//s = T(r) = 255*(n0 + n1 + ... + nr)/(width*height)
	if (!getHistogram())return 0;
	int32_t** sum = new int32_t * [3]{ new int32_t[256](),new int32_t[256]() ,new int32_t[256]() };
	for (int32_t i = 0;i < 3;++i)
	{
		sum[i][0] = histogram[i][0];
		for (int32_t j = 1;j < 256;++j)
		{
			sum[i][j] = sum[i][j - 1] + histogram[i][j];
		}
	}
	if (isGray)
	{
		for (int32_t i = 0;i < height;++i)
		{
			for (int32_t j = 0;j < width;++j)
			{
				pixels[i][j] = PIXEL(255 * (double)sum[0][pixels[i][j].blue] / ((double)width * (double)height));
			}
		}
	}
	else
	{
		for (int32_t i = 0;i < height;++i)
		{
			for (int32_t j = 0;j < width;++j)
			{
				pixels[i][j] = PIXEL(
					255 * (double)sum[0][pixels[i][j].blue] / ((double)width * (double)height),
					255 * (double)sum[1][pixels[i][j].green] / ((double)width * (double)height),
					255 * (double)sum[2][pixels[i][j].red] / ((double)width * (double)height)
				);
			}
		}
	}
	updateHistogram();
	for (int32_t i = 0;i < 3;++i)
	{
		delete[] sum[i];
	}
	delete[] sum;
	return 1;
}

/******************************************************************************************************
spatial filter
*******************************************************************************************************/

bool bmpMat::meanFilter(int32_t n)
{
	if (n < 0 || (n & 1) == 0)return 0;
	//get coefficient matrix
	double** matrix = new double* [n];
	for (int32_t i = 0;i < n;++i)
	{
		matrix[i] = new double[n];
		for (int32_t j = 0;j < n;++j)
		{
			matrix[i][j] = 1 / ((double)n * (double)n);
		}
	}
	PIXEL** buffer = new PIXEL*[height];
	for (int32_t i = 0;i < height;++i)
	{
		buffer[i] = new PIXEL[width];
	}
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			double sumBlue = 0, sumGreen = 0, sumRed = 0;
			for (int32_t ii = i - n / 2, iii = 0;ii <= i + n / 2;++ii, ++iii)
			{
				for (int32_t jj = j - n / 2, jjj = 0;jj <= j + n / 2;++jj, ++jjj)
				{
					
					if (ii >= 0 && jj >= 0 && ii < height && jj < width)
					{
						sumBlue += matrix[iii][jjj] * (double)pixels[ii][jj].blue;
						sumGreen += matrix[iii][jjj] * (double)pixels[ii][jj].green;
						sumRed += matrix[iii][jjj] * (double)pixels[ii][jj].red;
					}
				}
			}
			buffer[i][j] = PIXEL(
				(BYTE)sumBlue,
				(BYTE)sumGreen,
				(BYTE)sumRed
			);
		}
	}
	for (int32_t i = 0;i < height;++i)
	{
		delete[] pixels[i];
	}
	delete[] pixels;
	pixels = buffer;
	updateHistogram();
	for (int32_t i = 0;i < n;++i)
	{
		delete[] matrix[i];
	}
	delete[] matrix;
	return 1;
}

bool bmpMat::gaussFilter(int32_t n)
{
	if (n < 0 || (n & 1) == 0)return 0;
	//get coefficient matrix
	double** matrix = new double* [n];
	double sum = 0;
	for (int32_t i = 0;i < n;++i)
	{
		matrix[i] = new double[n];
		for (int32_t j = 0;j < n;++j)
		{
			int32_t ii = i > (n / 2) ? n - i - 1 : i,
				jj = j > (n / 2) ? n - j - 1 : j;
			matrix[i][j] = (int64_t)1 << (ii + jj);
			sum += matrix[i][j];
		}
	}
	for (int32_t i = 0;i < n;++i)
	{
		for (int32_t j = 0;j < n;++j)
		{
			matrix[i][j] /= sum;
		}
	}

	PIXEL** buffer = new PIXEL * [height];
	for (int32_t i = 0;i < height;++i)
	{
		buffer[i] = new PIXEL[width];
	}
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			double sumBlue = 0, sumGreen = 0, sumRed = 0;
			for (int32_t ii = i - n / 2, iii = 0;ii <= i + n / 2;++ii, ++iii)
			{
				for (int32_t jj = j - n / 2, jjj = 0;jj <= j + n / 2;++jj, ++jjj)
				{

					if (ii >= 0 && jj >= 0 && ii < height && jj < width)
					{
						sumBlue += matrix[iii][jjj] * (double)pixels[ii][jj].blue;
						sumGreen += matrix[iii][jjj] * (double)pixels[ii][jj].green;
						sumRed += matrix[iii][jjj] * (double)pixels[ii][jj].red;
					}
				}
			}
			buffer[i][j] = PIXEL(
				(BYTE)sumBlue,
				(BYTE)sumGreen,
				(BYTE)sumRed
			);
		}
	}
	for (int32_t i = 0;i < height;++i)
	{
		delete[] pixels[i];
	}
	delete[] pixels;
	pixels = buffer;
	updateHistogram();
	for (int32_t i = 0;i < n;++i)
	{
		delete[] matrix[i];
	}
	delete[] matrix;
	return 1;
}

bool bmpMat::medianFilter(int32_t n)
{
	if (n < 0 || (n & 1) == 0)return 0;
	PIXEL** buffer = new PIXEL * [height];
	for (int32_t i = 0;i < height;++i)
	{
		buffer[i] = new PIXEL[width];
	}
	BYTE** buf = new BYTE * [3]{ new BYTE[(int64_t)n * (int64_t)n],
		new BYTE[(int64_t)n * (int64_t)n],
		new BYTE[(int64_t)n * (int64_t)n] };
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			int32_t index = 0;
			for (int32_t ii = i - n / 2;ii <= i + n / 2;++ii)
			{
				for (int32_t jj = j - n / 2;jj <= j + n / 2;++jj)
				{
					if (ii >= 0 && jj >= 0 && ii < height && jj < width)
					{
						buf[0][index] = pixels[ii][jj].blue;
						buf[1][index] = pixels[ii][jj].green;
						buf[2][index] = pixels[ii][jj].red;
					}
					else
					{
						buf[0][index] = buf[0][index] = buf[0][index] = 0;
					}
					++index;
				}
			}
			sort(buf[0], n * n);
			sort(buf[1], n * n);
			sort(buf[2], n * n);
			buffer[i][j] = PIXEL(
				buf[0][n * n / 2], 
				buf[1][n * n / 2],
				buf[2][n * n / 2]
			);
		}
	}
	for (int32_t i = 0;i < 3;++i)
	{
		delete[] buf[i];
	}
	delete[] buf;
	for (int32_t i = 0;i < height;++i)
	{
		delete[] pixels[i];
	}
	delete[] pixels;
	pixels = buffer;
	updateHistogram();
	return 1;
}

bool bmpMat::laplaceFilter()
{
	int32_t** matrix = new int32_t * [3]{ new int32_t[3]{1,1,1},new int32_t[3]{1,-8,1} ,new int32_t[3]{1,1,1} };
	PIXEL** buffer = new PIXEL * [height];
	for (int32_t i = 0;i < height;++i)
	{
		buffer[i] = new PIXEL[width];
	}
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			int32_t sumBlue = 0, sumGreen = 0, sumRed = 0;
			for (int32_t ii = i - 1, iii = 0;ii <= i + 1;++ii, ++iii)
			{
				for (int32_t jj = j - 1, jjj = 0;jj <= j + 1;++jj, ++jjj)
				{
					if (ii >= 0 && jj >= 0 && ii < height && jj < width)
					{
						sumBlue += matrix[iii][jjj] * (int32_t)pixels[ii][jj].blue;
						sumGreen += matrix[iii][jjj] * (int32_t)pixels[ii][jj].green;
						sumRed += matrix[iii][jjj] * (int32_t)pixels[ii][jj].red;
					}
				}
			}
			buffer[i][j] = PIXEL(
				byteLimited(sumBlue),
				byteLimited(sumGreen),
				byteLimited(sumRed)
			);
		}
	}
	for (int32_t i = 0;i < 3;++i)
	{
		delete[] matrix[i];
	}
	delete[] matrix;
	for (int32_t i = 0;i < height;++i)
	{
		delete[] pixels[i];
	}
	delete[] pixels;
	pixels = buffer;
	updateHistogram();
	return 1;
}

bool bmpMat::laplaceSharpen()
{
	int32_t** matrix = new int32_t * [3]{ new int32_t[3]{1,1,1},new int32_t[3]{1,-8,1} ,new int32_t[3]{1,1,1} };
	PIXEL** buffer = new PIXEL * [height];
	for (int32_t i = 0;i < height;++i)
	{
		buffer[i] = new PIXEL[width];
	}
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			int32_t sumBlue = 0, sumGreen = 0, sumRed = 0;
			for (int32_t ii = i - 1, iii = 0;ii <= i + 1;++ii, ++iii)
			{
				for (int32_t jj = j - 1, jjj = 0;jj <= j + 1;++jj, ++jjj)
				{
					if (ii >= 0 && jj >= 0 && ii < height && jj < width)
					{
						sumBlue += matrix[iii][jjj] * (int32_t)pixels[ii][jj].blue;
						sumGreen += matrix[iii][jjj] * (int32_t)pixels[ii][jj].green;
						sumRed += matrix[iii][jjj] * (int32_t)pixels[ii][jj].red;
					}
				}
			}
			buffer[i][j] = pixels[i][j] - PIXEL(
				byteLimited(sumBlue),
				byteLimited(sumGreen),
				byteLimited(sumRed)
			);
		}
	}
	for (int32_t i = 0;i < 3;++i)
	{
		delete[] matrix[i];
	}
	delete[] matrix;
	for (int32_t i = 0;i < height;++i)
	{
		delete[] pixels[i];
	}
	delete[] pixels;
	pixels = buffer;
	updateHistogram();
	return 1;
}

bool bmpMat::unsharpMask(int32_t n)
{
	if (n < 0 || (n & 1) == 0)return 0;
	highFrequrncyEmphasis(n, 1);
	return 1;
}

bool bmpMat::highFrequrncyEmphasis(int32_t n, int32_t k)
{
	if (k < 1|| n < 0 || (n & 1) == 0)return 0;
	PIXEL** buffer = new PIXEL * [height];
	for (int32_t i = 0;i < height;++i)
	{
		buffer[i] = new PIXEL[width];
		for (int32_t j = 0;j < width;++j)
		{
			buffer[i][j] = pixels[i][j];
		}
	}
	gaussFilter(n);
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			pixels[i][j] = buffer[i][j] - pixels[i][j];
			pixels[i][j] = buffer[i][j] + (pixels[i][j] * k);
		}
	}
	for (int32_t i = 0;i < height;++i)
	{
		delete[] buffer[i];
	}
	delete[] buffer;
	updateHistogram();
	return 1;
}

bool bmpMat::sobelFilter()
{
	int32_t** matrix1 = new int32_t * [3]{
		new int32_t[3]{-1,-2,-1},
		new int32_t[3]{ 0,0,0 },
		new int32_t[3]{ 1,2,1 },
	}, ** matrix2 = new int32_t * [3]{
		new int32_t[3]{-1,0,1},
		new int32_t[3]{ -2,0,2 },
		new int32_t[3]{ -1,0,1 },
	};
	/*
	int32_t** matrix1 = new int32_t * [3]{
	new int32_t[3]{-3,-10,-3},
	new int32_t[3]{ 0,0,0 },
	new int32_t[3]{3,10,3 },
	}, ** matrix2 = new int32_t * [3]{
		new int32_t[3]{-3,0,3},
		new int32_t[3]{ -10,0,10 },
		new int32_t[3]{ -3,0,3 },
	};
	*/
	PIXEL** buffer = new PIXEL * [height];
	for (int32_t i = 0;i < height;++i)
	{
		buffer[i] = new PIXEL[width];
		for (int32_t j = 0;j < width;++j)
		{
			buffer[i][j] = pixels[i][j];
		}
	}
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			int32_t sum[2][3]{ 0,0,0,0,0,0 };
			for (int32_t ii = i - 1, iii = 0;ii <= i + 1;++ii, ++iii)
			{
				for (int32_t jj = j - 1, jjj = 0;jj <= j + 1;++jj, ++jjj)
				{
					if (ii >= 0 && jj >= 0 && ii < height && jj < width)
					{
						sum[0][0] += matrix1[iii][jjj] * (int32_t)buffer[ii][jj].blue;
						sum[0][1] += matrix1[iii][jjj] * (int32_t)buffer[ii][jj].green;
						sum[0][2] += matrix1[iii][jjj] * (int32_t)buffer[ii][jj].red;
						sum[1][0] += matrix2[iii][jjj] * (int32_t)buffer[ii][jj].blue;
						sum[1][1] += matrix2[iii][jjj] * (int32_t)buffer[ii][jj].green;
						sum[1][2] += matrix2[iii][jjj] * (int32_t)buffer[ii][jj].red;
					}
				}
			}
			pixels[i][j] = PIXEL(
				byteLimited(abs(sum[0][0]) + abs(sum[1][0])),
				byteLimited(abs(sum[0][1]) + abs(sum[1][1])),
				byteLimited(abs(sum[0][2]) + abs(sum[1][2]))
			);
		}
	}
	updateHistogram();
	for (int32_t i = 0;i < 3;++i)
	{
		delete[] matrix1[i], matrix2[i];
	}
	delete[] matrix1, matrix2;
	for (int32_t i = 0;i < height;++i)
	{
		delete[] buffer[i];
	}
	delete[] buffer;
	return 1;
}

bool bmpMat::sharpen()
{
	bmpMat* gauss = new bmpMat(*this),
		* laplace = new bmpMat(*this);
	gauss->sobelFilter();
	gauss->gaussFilter(5);
	laplace->laplaceSharpen();
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			pixels[i][j] += (gauss->pixels[i][j] * laplace->pixels[i][j]);
		}
	}
	//gamma(0.5);
	histigramEqualization();
	delete gauss, laplace;
	return 1;
}

PIXEL bmpMat::moore()
{
	PIXEL dominate = pixels[0][0];
	int32_t count = 1;
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			if (pixels[i][j] != dominate)
			{
				--count;
				if (!count)
				{
					dominate = pixels[i][j];
					++count;
				}
			}
			else
			{
				++count;
			}
		}
	}
	return dominate;
}

bmpMat* bmpMat::getROI(int32_t x, int32_t y, int32_t h, int32_t w)
{
	if (x<0 || y<0 || x>height - h || y>width - w)return NULL;
	bmpMat* roi = new bmpMat(w, h);
	for (int32_t i = 0;i < w;++i)
	{
		for (int32_t j = 0;j < h;++j)
		{
			roi->pixels[i][j] = pixels[x+i][y+j];
		}
	}
	return roi;
}

bmpMat* bmpMat::getNotROI(int32_t x, int32_t y, int32_t h, int32_t w)
{
	if (x<0 || y<0 || x>height - h || y>width - w)return NULL;
	bmpMat* roi = new bmpMat(*this);
	for (int32_t i = 0;i < w;++i)
	{
		for (int32_t j = 0;j < h;++j)
		{
			roi->pixels[x + i][y + j] = PIXEL(0,0,0);
		}
	}
	return roi;
}

bmpMat* bmpMat::getMask(uint8_t threshold)
{
	bmpMat* mask = new bmpMat(*this);
	PIXEL dominate = moore();
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			if ((mask->pixels[i][j], dominate) < threshold)
			{
				mask->pixels[i][j] = PIXEL(0);
			}
			else
			{
				mask->pixels[i][j] = PIXEL(255);
			}
		}
	}
	return mask;
}

bool bmpMat::getFuse(bmpMat* foreground, bmpMat* mask)
{
	if (foreground->height != height || foreground->width != width ||
		mask->height != height || mask->width != width)return 0;
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			if (mask->pixels[i][j].blue)
			{
				pixels[i][j] = foreground->pixels[i][j];
			}
		}
	}
	return 1;
}

bool bmpMat::getFuse(bmpMat* foreground, bmpMat* mask, int32_t x, int32_t y)
{
	if (x + foreground->height >= this->height || y + foreground->width >= this->width)
	{
		return 0;
	}
	int32_t h = foreground->height;
	int32_t w = foreground->width;
	for (int32_t i = 0;i < h;++i)
	{
		for (int32_t j = 0;j < w;++j)
		{
			/*if (mask->pixels[i][j].blue)
			{
				pixels[x + i][y + j] = foreground->pixels[i][j];
			}*/
			double weight_foreground = (mask->pixels[i][j].blue)/255;
			double weight_background = 1 - weight_foreground;
			pixels[x + i][y + j] = foreground->pixels[i][j] * weight_foreground + pixels[x + i][y + j] * weight_background;
		}
	}
	return 1;
}
