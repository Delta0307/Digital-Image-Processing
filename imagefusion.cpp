#include "imagefusion.h"

#define PATH1 "E:\\lzu\\graduationproject\\lena.bmp"
#define PATH2 "E:\\lzu\\graduationproject\\cameraman.bmp"
#define PATH3 "E:\\lzu\\graduationproject\\mandril.bmp"
#define RES1 "E:\\lzu\\graduationproject\\result1.bmp"
#define RES2 "E:\\lzu\\graduationproject\\result2.bmp"

#define FOREGROUND "E:\\lzu\\graduationproject\\doctor2.bmp"
#define BACKGROUND "E:\\lzu\\graduationproject\\street.bmp"

int main()
{
	//色差法
	bmpMat* mat1 = new bmpMat(FOREGROUND), * mat2 = new bmpMat(BACKGROUND);
	auto mat3 = mat1->getMask(100);
	mat3->meanFilter(3);
	//mat3->laplaceSharpen();
	//mat3->gaussFilter(3);
	mat2->getFuse(mat1, mat3, 50, 200);
	bmpMat::writeBmp(mat2, RES1);

	delete mat1, mat2;
	return 0;
}
