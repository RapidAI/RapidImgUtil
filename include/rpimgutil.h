#pragma once 
#include "opencv2/imgproc.hpp"
#include "FreeImage.h"
typedef FREE_IMAGE_FORMAT RPIMGTYPE;

typedef struct { uint32_t magic; RPIMGTYPE Type; } RP_IMG_MAP;

extern RP_IMG_MAP QMIMGMAPTABLE[];
class CRpImgRead
{

public:

	static cv::Mat		RpReadImg(const BYTE* pImgBuf, uint32_t nBufLen);
	static RPIMGTYPE	RpGetImgType(const BYTE* pImgBuf, uint32_t nBufLen);
	static  bool		RpGetImgSize(const BYTE* pImgBuf, uint32_t nBufLen, int& nWidth, int& nHeight);

	static cv::Mat  RpRotateImg(cv::Mat& src, double angle = 45.0);
	static cv::Mat	bitMap2Mat(FIBITMAP* image, bool bFree=true);
	static void		RpImageInit();
	static void		RpImageDeInit();
	static cv::Mat  RpReadImgFromMem(FREE_IMAGE_FORMAT format, const unsigned char* sGifBuf, int nLen);
	static cv::Mat  RpReadTiffFromMem(FREE_IMAGE_FORMAT format, const unsigned char* sGifBuf, int nLen);
	static cv::Mat  RpReadGIFFile(const char* szGIFPath);
	static cv::Mat  RpReadTifFile(const char* szGIFPath);
	static cv::Mat	RpReadTGAMem(const unsigned char* ImgBuf, int nLen);
	static cv::Mat	RpReadWebpMem(const unsigned char* pBuf, int nLen);
	static cv::Mat	clahe_deal(cv::Mat& src, float fStrength);  // augment contrast of a image.
	static std::vector<cv::Mat> RpReadGifAllMem(const unsigned char* pGifBuf, int nLen);


};