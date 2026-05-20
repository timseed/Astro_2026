#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

#include "Image.h"

extern bool 
ReadImage(const char *filename, int &width, int &height, 
          unsigned char *&rgb_data, unsigned char *&png_alpha);

extern bool 
WriteImage(const char *filename, const int width, const int height, 
           unsigned char * const rgb_data, 
           unsigned char * const png_alpha, 
           const int quality);

Image::Image() : width_(0), height_(0), area_(0), 
                 rgbData_(NULL), pngAlpha_(NULL), quality_(80)
{
}

Image::Image(const int w, const int h, const unsigned char *rgb, 
             const unsigned char *alpha) 
    : width_(w), height_(h), area_(w*h), quality_(80)
{
    rgbData_ = (unsigned char *) malloc(3 * area_);
    memcpy(rgbData_, rgb, 3 * area_);

    if (alpha == NULL)
    {
        pngAlpha_ = NULL;
    }
    else
    {
        pngAlpha_ = (unsigned char *) malloc(area_);
        memcpy(pngAlpha_, alpha, area_);
    }
}

Image::~Image()
{
    free(rgbData_);
    free(pngAlpha_);
}

bool
Image::Read(const char *filename)
{
    bool success = ReadImage(filename, width_, height_, rgbData_, pngAlpha_);
    area_ = width_ * height_;
    return(success);
}

bool
Image::Write(const char *filename)
{
    bool success = WriteImage(filename, width_, height_, rgbData_, pngAlpha_, 
                              quality_);
    return(success);
}

bool
Image::Crop(const int x0, const int y0, const int x1, const int y1)
{
    if (x1 <= x0 || y1 <= y0) return(false);

    const int w = x1 - x0;
    const int h = y1 - y0;
    const int new_area = w * h;

    unsigned char *new_rgb = (unsigned char *) malloc(3 * new_area);
    memset(new_rgb, 0, 3 * new_area);

    unsigned char *new_alpha = NULL;
    if (pngAlpha_ != NULL)
    {
        new_alpha = (unsigned char *) malloc(new_area);
        memset(new_alpha, 0, new_area);
    }

    int ipos = 0;
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            memcpy(new_rgb + ipos,
                   rgbData_ + 3 * ((j + y0) * width_ + (i + x0)), 3);
            ipos += 3;
        }
    }
    free(rgbData_);
    free(pngAlpha_);

    width_    = w;
    height_   = h;
    area_     = new_area;
    rgbData_  = new_rgb;
    pngAlpha_ = new_alpha;

    return(true);
}

void
Image::Reduce(const int factor)
{
    if (factor < 1) return;

    int scale = 1;
    for (int i = 0; i < factor; i++) scale *= 2;

    double scale2 = scale*scale;

    int w = width_ / scale;
    int h = height_ / scale;
    int new_area = w * h;

    unsigned char *new_rgb = (unsigned char *) malloc(3 * new_area);
    memset(new_rgb, 0, 3 * new_area);

    unsigned char *new_alpha = NULL;
    if (pngAlpha_ != NULL)
    {
        new_alpha = (unsigned char *) malloc(new_area);
        memset(new_alpha, 0, new_area);
    }

    int ipos = 0;
    for (int j = 0; j < height_; j++)
    {
        int js = j / scale;
        for (int i = 0; i < width_; i++)
        {
            int is = i/scale;
            for (int k = 0; k < 3; k++)
                new_rgb[3*(js * w + is) + k] += static_cast<unsigned char> ((rgbData_[3*ipos + k] + 0.5) / scale2);

            if (pngAlpha_ != NULL) 
                new_alpha[js * w + is] += static_cast<unsigned char> (pngAlpha_[ipos]/scale2);
            ipos++;
        }
    }

    free(rgbData_);
    free(pngAlpha_);

    width_    = w;
    height_   = h;
    area_     = new_area;
    rgbData_  = new_rgb;
    pngAlpha_ = new_alpha;
}

void
Image::Resize(const int w, const int h)
{
    int new_area = w * h;
    
    unsigned char *new_rgb = (unsigned char *) malloc(3 * new_area);
    unsigned char *new_alpha = NULL;
    if (pngAlpha_ != NULL)
        new_alpha = (unsigned char *) malloc(new_area);

    const double scale_x = ((double) w) / width_;
    const double scale_y = ((double) h) / height_;
    
    int ipos = 0;
    for (int j = 0; j < h; j++)
    {
        const double y = j / scale_y;
        for (int i = 0; i < w; i++)
        {
            const double x = i / scale_x;
            if (new_alpha == NULL)
                getPixel(x, y, new_rgb + 3*ipos);
            else
                getPixel(x, y, new_rgb + 3*ipos, new_alpha + ipos);
            ipos++;
        }
    }

    free(rgbData_);
    free(pngAlpha_);

    width_    = w;
    height_   = h;
    area_     = w * h;
    rgbData_  = new_rgb;
    pngAlpha_ = new_alpha;
}

// Slide the whole image to the right (for positive x) or to the left
// (for negative x).  The original pixel 0 column will be in the pixel
// x column after this routine is called.
void
Image::Shift(const int x)
{
    unsigned char *new_rgb = (unsigned char *) malloc(3 * area_);
    unsigned char *new_alpha = NULL;
    if (pngAlpha_ != NULL)
        new_alpha = (unsigned char *) malloc(area_);

    int shift = x;
    while (shift < 0)
        shift += width_;
    while (shift >= width_)
        shift -= width_;

    int ipos = 0;
    for (int j = 0; j < height_; j++)
    {
        for (int i = 0; i < width_; i++)
        {
            int ii = i + shift;
            if (ii < 0) ii += width_;
            if (ii >= width_) ii -= width_;

            int iipos = j * width_ + ii;
            memcpy(new_rgb + 3*ipos, rgbData_ + 3*iipos, 3);

            if (pngAlpha_ != NULL)
                new_alpha[ipos] = pngAlpha_[iipos];

            ipos++;
        }
    }
    free(rgbData_);
    free(pngAlpha_);

    rgbData_  = new_rgb;
    pngAlpha_ = new_alpha;
}

// Find the color of the desired point using bilinear interpolation.
// Assume the array indices refer to the center of the pixel, so each
// pixel has corners at (i - 0.5, j - 0.5) and (i + 0.5, j + 0.5)
void
Image::getPixel(double x, double y, unsigned char *pixel)
{
    getPixel(x, y, pixel, NULL);
}

void
Image::getPixel(double x, double y, unsigned char *pixel, 
                unsigned char *alpha)
{
    if (x < -0.5) x = -0.5;
    if (x >= width_ - 0.5) x = width_ - 0.5;

    if (y < -0.5) y = -0.5;
    if (y >= height_ - 0.5) y = height_ - 0.5;

    int ix0 = (int) (floor(x));
    int ix1 = ix0 + 1;
    if (ix0 < 0) ix0 = width_ - 1;
    if (ix1 >= width_) ix1 = 0;

    int iy0 = (int) (floor(y));
    int iy1 = iy0 + 1;
    if (iy0 < 0) iy0 = 0;
    if (iy1 >= height_) iy1 = height_ - 1;

    const double t = x - floor(x);
    const double u = 1 - (y - floor(y));

    // Weights are from Numerical Recipes, 2nd Edition
    //        weight[0] = (1 - t) * u;
    //        weight[2] = (1-t) * (1-u);
    //        weight[3] = t * (1-u);
    double weight[4];
    weight[1] = t * u;
    weight[0] = u - weight[1];
    weight[2] = 1 - t - u + weight[1];
    weight[3] = t - weight[1];

    unsigned char *pixels[4];
    pixels[0] = rgbData_ + 3 * (iy0 * width_ + ix0);
    pixels[1] = rgbData_ + 3 * (iy0 * width_ + ix1);
    pixels[2] = rgbData_ + 3 * (iy1 * width_ + ix0);
    pixels[3] = rgbData_ + 3 * (iy1 * width_ + ix1);

    memset(pixel, 0, 3);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
            pixel[j] += (unsigned char) (weight[i] * pixels[i][j]);
    }

    if (alpha != NULL)
    {
        unsigned char pixels[4];
        pixels[0] = pngAlpha_[iy0 * width_ + ix0];
        pixels[1] = pngAlpha_[iy0 * width_ + ix1];
        pixels[2] = pngAlpha_[iy0 * width_ + ix0];
        pixels[3] = pngAlpha_[iy1 * width_ + ix1];

        *alpha = 0;
        for (int i = 0; i < 4; i++)
            *alpha += (unsigned char) (weight[i] * pixels[i]);
    }
}
