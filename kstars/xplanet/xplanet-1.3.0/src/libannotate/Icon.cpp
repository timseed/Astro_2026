#include <cstring>
#include <sstream>
using namespace std;

#include "findFile.h"
#include "xpUtil.h"

#include "Icon.h"

#include "libdisplay/libdisplay.h"
#include "libimage/Image.h"

Icon::Icon(const int x, const int y, const std::string &filename,
           const unsigned char *transparent)
    : x_(x), y_(y), filename_(filename), image_(NULL), transparent_(NULL)
{
    bool foundFile = findFile(filename_, "images");
    if (foundFile)
    { 
        image_ = new Image;
        foundFile = image_->Read(filename_.c_str());
    
        width_ = image_->Width();
        height_ = image_->Height();
        
        if (transparent != NULL)
        {
            transparent_ = new unsigned char[3];
            memcpy(transparent_, transparent, 3);
        }
    }
    else
    {
        ostringstream errStr;
        errStr << "Can't find image file " << filename << endl;
        xpWarn(errStr.str(), __FILE__, __LINE__);
    }
}

Icon::~Icon()
{
    delete image_;

    delete [] transparent_;
}

void
Icon::Draw(DisplayBase *display)
{
    if (image_ != NULL)
    {
        const unsigned char *rgb_data = image_->getRGBData();
        const unsigned char *png_alpha = image_->getPNGAlpha();

        const int ulx = x_ - width_ / 2;
        const int uly = y_ - height_ / 2;

        for (int j = 0; j < height_; j++)
        {
            for (int i = 0; i < width_; i++)
            {
                const unsigned char *pixel = rgb_data + 3*(j * width_ + i);

                double opacity = 1;
                if (transparent_ != NULL)
                {
                    opacity = 0;
                    for (int ii = 0; ii < 3; ii++)
                    {
                        if (pixel[ii] != transparent_[ii])
                        {
                            opacity = 1;
                            break;
                        }
                    }
                }
                else if (png_alpha != NULL)
                {
                    opacity = png_alpha[j * width_ + i]  / 255.;
                }

                display->setPixel(ulx + i, uly + j, pixel, opacity);
            }
        }
    }
}

