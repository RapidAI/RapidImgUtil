#include "precomp.h"


RPIMGTYPE CRpImgRead::RpGetImgType(const BYTE* pImgBuf, uint32_t nBufLen)
{

    FIMEMORY* memory = FreeImage_OpenMemory((BYTE*)pImgBuf, nBufLen);
    if (!memory)
        return FIF_UNKNOWN;
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(memory, 0);

    FreeImage_CloseMemory(memory);
    return fif;
}



cv::Mat  CRpImgRead::RpRotateImg(Mat& src, double angle)
{

    Mat out_img;


    // 计算旋转后输出图形的尺寸
    int rotated_width = (int)ceil(src.rows * fabs(sin(angle * CV_PI / 180)) + src.cols * fabs(cos(angle * CV_PI / 180)));
    int rotated_height = (int)ceil(src.cols * fabs(sin(angle * CV_PI / 180)) + src.rows * fabs(cos(angle * CV_PI / 180)));

    // 计算仿射变换矩阵
    Point2f center((float)src.cols / 2, (float)src.rows / 2);
    Mat rotate_matrix = cv::getRotationMatrix2D(center, angle, 1.0);

    // 防止切边，对平移矩阵B进行修改
    rotate_matrix.at<double>(0, 2) += (rotated_width - src.cols) / 2;
    rotate_matrix.at<double>(1, 2) += (rotated_height - src.rows) / 2;

    // 应用仿射变换
    cv::warpAffine(src, out_img, rotate_matrix, Size(rotated_width, rotated_height), INTER_LINEAR, 0, Scalar(255, 255, 255));

    return out_img;
}



cv::Mat CRpImgRead::bitMap2Mat(FIBITMAP* image,bool bFree)
{
    
    if (FreeImage_GetBPP(image) != 8)
        image = FreeImage_ConvertTo8Bits(image);

    RGBQUAD* pal = FreeImage_GetPalette(image);
    unsigned width = FreeImage_GetWidth(image);
    unsigned height = FreeImage_GetHeight(image);

    FIBITMAP* tmp = FreeImage_Allocate(height, width, 24);
    BYTE* tmp_bits = FreeImage_GetBits(tmp);

    Mat img = Mat::zeros(height, width, CV_8UC3);
    BYTE Iintensity;
    uchar* p;

    for (unsigned int i = 0; i < height; i++)
    {
        p = img.ptr<uchar>(i);
        for (unsigned int j = 0; j < width; j++)
        {
            FreeImage_GetPixelIndex(image, j, height - i, &Iintensity);
            p[3 * j] = pal[Iintensity].rgbBlue;
            p[3 * j + 1] = pal[Iintensity].rgbGreen;
            p[3 * j + 2] = pal[Iintensity].rgbRed;
        }
    }
    if (tmp)
        FreeImage_Unload(tmp);
   
    if(bFree)
     FreeImage_Unload(image);

    return img;
}



cv::Mat  CRpImgRead::RpReadImgFromMem(FREE_IMAGE_FORMAT format, const unsigned char* sGifBuf, int nLen)
{
    cv::Mat dst;
    FIMEMORY* memory = FreeImage_OpenMemory((BYTE*)sGifBuf, nLen);
    if (!memory)
        return dst;
    FIBITMAP* image = FreeImage_LoadFromMemory(format, memory, 0);
     dst = bitMap2Mat(image);
    FreeImage_CloseMemory(memory);
    return dst;
}

cv::Mat  CRpImgRead::RpReadTiffFromMem(FREE_IMAGE_FORMAT format, const unsigned char* sGifBuf, int nLen)
{
    cv::Mat dst;
    FIMEMORY* memory = FreeImage_OpenMemory((BYTE*)sGifBuf, nLen);
    if (!memory)
        return dst;
    FIBITMAP* image = FreeImage_LoadFromMemory(format, memory, 0);
    dst = bitMap2Mat(image);
    FreeImage_CloseMemory(memory);
    return dst;
}



cv::Mat  CRpImgRead::RpReadGIFFile(const char* szGIFPath)
{
    FIBITMAP* image = FreeImage_Load(FIF_GIF, szGIFPath);
    return bitMap2Mat(image);
}
cv::Mat  CRpImgRead::RpReadTifFile(const char* szGIFPath)
{
    FIBITMAP* image = FreeImage_Load(FIF_TARGA, szGIFPath);
    return bitMap2Mat(image);
}

void  CRpImgRead::RpImageInit()
{

    FreeImage_Initialise();
}

void  CRpImgRead::RpImageDeInit()
{
    FreeImage_DeInitialise();
}

cv::Mat CRpImgRead::RpReadTGAMem(const unsigned char* ImgBuf, int nLen)
{

    Tga tgaImg((char*)ImgBuf, nLen);
    Mat img(tgaImg.GetHeight(), tgaImg.GetWidth(), CV_8UC4);
    memcpy(img.data, tgaImg.GetPixels().data(), tgaImg.GetHeight() * tgaImg.GetWidth() * 4);


    Mat flipImg, RGBImg;
    cv::cvtColor(img, RGBImg, COLOR_BGRA2BGR);
    cv::flip(RGBImg, flipImg, 1);
    Point center(flipImg.cols / 2, flipImg.rows / 2); //旋转中心
    Mat rotMat = getRotationMatrix2D(center, 180.0, 1);
    warpAffine(flipImg, img, rotMat, flipImg.size());

   // imwrite("testbak.jpg", img);
    return img;
}


cv::Mat CRpImgRead::RpReadWebpMem(const unsigned char* pBuf, int nLen)
{
    cv::_InputArray pic_arr(pBuf, nLen);
    cv::Mat img_encode;
    img_encode = imdecode(pic_arr, cv::IMREAD_UNCHANGED);
    std::vector<uchar> data_encode;
    std::vector<int> param = std::vector<int>(2);
    param[0] = cv::IMWRITE_WEBP_QUALITY;
    param[1] = 100;//default(95) 0-100
    imencode(".webp", img_encode, data_encode, param);
    std::string str_encode(data_encode.begin(), data_encode.end());
    //decode image
    Mat img_decode;
    img_decode = imdecode(data_encode, cv::IMREAD_COLOR);
    return img_decode;
}




cv::Mat CRpImgRead::RpReadImg(const BYTE* pImgBuf, uint32_t nBufLen)
{

    cv::Mat oriImg;
    RPIMGTYPE Type = RpGetImgType(pImgBuf, nBufLen);

    switch (Type)
    {
    case FIF_GIF:
        return RpReadImgFromMem(FIF_GIF, pImgBuf, nBufLen);

    case FIF_WEBP:
        return RpReadWebpMem(pImgBuf, nBufLen);
    case FIF_TARGA:

        return  RpReadTGAMem(pImgBuf, nBufLen);

    case FIF_TIFF:

        return RpReadTiffFromMem(FIF_TIFF,pImgBuf, nBufLen);

// other format to be preprocessed.
    default: // formats which can be processed by opencv;

        cv::_InputArray pic_arr(pImgBuf, nBufLen);
        oriImg = cv::imdecode(pic_arr, cv::IMREAD_COLOR);
        return oriImg;
        
    }
  
    
}



inline void color_transfer_with_spilt(cv::Mat& input, std::vector<cv::Mat>& chls)
{
    cv::cvtColor(input, input, cv::COLOR_BGR2YCrCb);
    cv::split(input, chls);
}

inline  void color_retransfer_with_merge(cv::Mat& output, std::vector<cv::Mat>& chls)
{
    cv::merge(chls, output);
    cv::cvtColor(output, output, cv::COLOR_YCrCb2BGR);
}

cv::Mat CRpImgRead::clahe_deal(cv::Mat& src, float fStrength)
{
    cv::Mat ycrcb = src.clone();
    std::vector<cv::Mat> channels;

    color_transfer_with_spilt(ycrcb, channels);

    cv::Mat clahe_img;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    // 直方图的柱子高度大于计算后的ClipLimit的部分被裁剪掉，然后将其平均分配给整张直方图
    // 从而提升整个图像
    clahe->setClipLimit(fStrength);    // (int)(4.*(8*8)/256)
    clahe->setTilesGridSize(Size(8, 8)); // 将图像分为8*8块
    clahe->apply(channels[0], clahe_img);
    channels[0].release();
    clahe_img.copyTo(channels[0]);
    color_retransfer_with_merge(ycrcb, channels);
    return ycrcb;

}



std::vector<cv::Mat> CRpImgRead::RpReadGifAllMem(const unsigned char* pGifBuf, int nLen)
{

    vector<cv::Mat> dst;
    FIMEMORY* memory = FreeImage_OpenMemory((BYTE*)pGifBuf, nLen);
    if (!memory)
        return dst;
    FIMULTIBITMAP* fimb = FreeImage_LoadMultiBitmapFromMemory(FIF_GIF, memory, GIF_DEFAULT);
    // frame count
    int image_count = FreeImage_GetPageCount(fimb);
    if (!image_count)
    {
        FreeImage_CloseMemory(memory);
        return dst;
    }

    // pre define size
    int width = 0, height = 0;
    // other buffer variables
    int mat_type = 0;
    int channels = 0;
//    RGBQUAD ptrPalette;
    Mat mat;

    for (int cur_frame = 0; cur_frame < image_count; cur_frame++) 
    {
        FIBITMAP* fib_frame = FreeImage_LockPage(fimb, cur_frame);

        
        mat = bitMap2Mat(fib_frame,false);
       
        FreeImage_UnlockPage(fimb, fib_frame,FALSE);

        dst.push_back(mat);

    }

    FreeImage_CloseMemory(memory);

    return dst;

}


 bool		CRpImgRead::RpGetImgSize(const BYTE* pImgBuf, uint32_t nBufLen, int& nWidth, int& nHeight)
{
    RPIMG::RPMIMG_STREAM  stream = { 0 };
    stream.data = (const char*)pImgBuf;
    stream.nLen = nBufLen;

    auto info = RPIMG::rpimg_getimagesize_from_stream(&stream);

    if (info)
    {

        nWidth = info->width;
        nHeight=info->height;
        rpimg_free_gfxinfo(info);

        return true;
    }

    return false;
}