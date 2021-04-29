#pragma once
#include <vector>
#include <fstream>

typedef union PixelInfo
{
    std::uint32_t Colour;
    struct
    {
        std::uint8_t R, G, B, A;
    };
} *PPixelInfo;

class Tga
{
private:
    std::vector<std::uint8_t> Pixels;
    bool ImageCompressed;
    std::uint32_t width, height, size, BitsPerPixel;

public:
    inline Tga(const char* pImgBuf, int nLen);
    std::vector<std::uint8_t> GetPixels() { return this->Pixels; }
    std::uint32_t GetWidth() const { return this->width; }
    std::uint32_t GetHeight() const { return this->height; }
    bool HasAlphaChannel() { return BitsPerPixel == 32; }
};

Tga::Tga(const char* pImgBuf, int nLen)
{

    uint32_t nOffset = 0;
   
    std::uint8_t* Header= (uint8_t * )pImgBuf;
    std::vector<std::uint8_t> ImageData;
    static std::uint8_t DeCompressed[12] = { 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    static std::uint8_t IsCompressed[12] = { 0x0, 0x0, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

#define TGA_HDR_SIZE 18

    nOffset += TGA_HDR_SIZE;
    if (!std::memcmp(DeCompressed, Header, sizeof(DeCompressed)))
    {
        BitsPerPixel = Header[16];
        width = Header[13] * 256 + Header[12];
        height = Header[15] * 256 + Header[14];
        size = ((width * BitsPerPixel + 31) / 32) * 4 * height;

        if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
        {

            throw std::invalid_argument("Invalid File Format. Required: 24 or 32 Bit Image.");
        }

        ImageData.resize(size);
        ImageCompressed = false;
        memcpy(reinterpret_cast<char*>(ImageData.data()), pImgBuf + nOffset, size);

        nOffset += size;
    }
    else if (!std::memcmp(IsCompressed, Header, sizeof(IsCompressed)))
    {
        BitsPerPixel = Header[16];
        width = Header[13] * 256 + Header[12];
        height = Header[15] * 256 + Header[14];
        size = ((width * BitsPerPixel + 31) / 32) * 4 * height;

        if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
        {
            
            throw std::invalid_argument("Invalid File Format. Required: 24 or 32 Bit Image.");
        }

        PixelInfo Pixel = { 0 };
        int CurrentByte = 0;
        std::size_t CurrentPixel = 0;
        ImageCompressed = true;
        std::uint8_t ChunkHeader = { 0 };
        int BytesPerPixel = (BitsPerPixel / 8);
        ImageData.resize(width * height * sizeof(PixelInfo));

        do
        {
            //hFile.read(reinterpret_cast<char*>(&ChunkHeader), sizeof(ChunkHeader));
            memcpy(reinterpret_cast<char*>(&ChunkHeader), pImgBuf + nOffset, sizeof(ChunkHeader));
            nOffset += sizeof(ChunkHeader);
            if (ChunkHeader < 128)
            {
                ++ChunkHeader;
                for (int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
                {
                    //hFile.read(reinterpret_cast<char*>(&Pixel), BytesPerPixel);
                    memcpy(reinterpret_cast<char*>(&Pixel), pImgBuf + nOffset, BytesPerPixel);
                    nOffset += BytesPerPixel;
                    ImageData[CurrentByte++] = Pixel.B;
                    ImageData[CurrentByte++] = Pixel.G;
                    ImageData[CurrentByte++] = Pixel.R;
                    if (BitsPerPixel > 24) ImageData[CurrentByte++] = Pixel.A;
                }
            }
            else
            {
                ChunkHeader -= 127;
               // hFile.read(reinterpret_cast<char*>(&Pixel), BytesPerPixel);
                memcpy(reinterpret_cast<char*>(&Pixel), pImgBuf + nOffset, BytesPerPixel);
                nOffset += BytesPerPixel;
                for (int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
                {
                    ImageData[CurrentByte++] = Pixel.B;
                    ImageData[CurrentByte++] = Pixel.G;
                    ImageData[CurrentByte++] = Pixel.R;
                    if (BitsPerPixel > 24) ImageData[CurrentByte++] = Pixel.A;
                }
            }
        } while (CurrentPixel < (width * height));
    }
    else
    {
        
        throw std::invalid_argument("Invalid File Format. Required: 24 or 32 Bit TGA File.");
    }

    
    this->Pixels = ImageData;
}



