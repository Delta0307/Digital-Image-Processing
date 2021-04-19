#pragma once
#include<stdint.h>

template<class TYPE = int32_t>
class mat
{
public:
	mat(int32_t h,int32_t w);
	~mat();
	int32_t getWidth();
	int32_t getHeight();
	void add(int32_t h, int32_t w, TYPE v);
	TYPE at(int32_t h, int32_t w);
	mat<TYPE>* mat<TYPE>::addMat(mat<TYPE>* b);
	mat<TYPE>* mat<TYPE>::subMat(mat<TYPE>* b);
	mat<TYPE>* mat<TYPE>::mulMat(mat<TYPE>* b);
	mat<TYPE>* mat<TYPE>::mul(TYPE k);
private:
	int32_t height;
	int32_t width;
	TYPE** value;
};

template<class TYPE>
mat<TYPE>::mat(int32_t h, int32_t w):height(h),width(w)
{
	value = new TYPE * [height];
	for (int32_t i = 0;i < height;++i)
	{
		value[i] = new TYPE[width];
		for (int32_t j = 0;j < width;++j)
		{
			value[i][j] = 0;
		}
	}
}

template<class TYPE>
mat<TYPE>::~mat()
{
	for (int32_t i = 0;i < height;++i)
	{
		delete[] value[i];
	}
	delete[] value;
}

template<class TYPE>
int32_t mat<TYPE>::getHeight()
{
	return height;
}

template<class TYPE>
int32_t mat<TYPE>::getWidth()
{
	return width;
}

template<class TYPE>
void mat<TYPE>::add(int32_t h, int32_t w, TYPE v)
{
	value[h][w] = v;
}

template<class TYPE>
inline TYPE mat<TYPE>::at(int32_t h, int32_t w)
{
	if (h < 0 || h >= height || w < 0 || w >= width)return 0;
	return value[h][w];
}

template<class TYPE>
inline mat<TYPE>* mat<TYPE>::addMat(mat<TYPE>* b)
{
	mat<TYPE>* res = new mat<TYPE>(height, b->getWidth());
	if (width != b->getHeight())return res;
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < b->getWidth();++j)
		{
			res->value[i][j] = value[i][j] + b->value[i][j];
		}
	}
	return res;
}

template<class TYPE>
inline mat<TYPE>* mat<TYPE>::subMat(mat<TYPE>* b)
{
	mat<TYPE>* res = new mat<TYPE>(height, b->getWidth());
	if (width != b->getHeight())return res;
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < b->getWidth();++j)
		{
			res->value[i][j] = value[i][j] - b->value[i][j];
		}
	}
	return res;
}

template<class TYPE>
inline mat<TYPE>* mat<TYPE>::mulMat(mat<TYPE>* b)
{
	mat<TYPE> *res = new mat<TYPE>(height, b.width);
	if (width != b.height)return res;
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < b.width;++j)
		{
			res->value[i][j] = 0;
			for (int32_t k = 0;k < width;++k)
			{
				res->value[i][j] = res->value[i][j] + value[i][k] * b.value[k][j];
			}
		}
	}
	return res;
}

template<class TYPE>
inline mat<TYPE>* mat<TYPE>::mul(TYPE k)
{
	mat<TYPE>* res = new mat<TYPE>(height, width);
	for (int32_t i = 0;i < height;++i)
	{
		for (int32_t j = 0;j < width;++j)
		{
			res->value[i][j] = k * value[i][j];
		}
	}
	return res;
}
