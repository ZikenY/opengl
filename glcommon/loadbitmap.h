#ifndef LOADBITMAP_H
#define LOADBITMAP_H

#include <cstdint>
#include <vector>
#include <fstream>

#ifdef _MSC_FULL_VER
#include "c:\\sdks\\glew\\include\\GL\\glew.h"
#endif
#ifdef __GNUC__
#include <GL/glu.h>
#endif

namespace glcommon
{

typedef union PixelInfo
{
    std::uint32_t Colour;
    struct
    {
        std::uint8_t B, G, R, A;
    };
} *PPixelInfo;

struct Texture
{
    std::vector<std::uint8_t> Pixels;
    std::uint32_t width, height, size, BitsPerPixel;
    bool HasAlphaChannel() { return BitsPerPixel == 32; }
};

struct Bitmap : public Texture
{
    Bitmap(const char* FilePath);
};

struct Tga : public Texture
{
    Tga(const char* FilePath);
    bool ImageCompressed;
    std::vector<std::uint8_t> GetPixels() { return this->Pixels; }
    std::uint32_t GetWidth() const {return this->width;}
    std::uint32_t GetHeight() const {return this->height;}
};

extern GLuint gentexture(Texture *texture, GLenum activetexture = GL_TEXTURE0);

}

#endif // LOADBITMAP_H
