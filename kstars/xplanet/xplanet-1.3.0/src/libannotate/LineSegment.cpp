#include <cmath>
using namespace std;

#include "LineSegment.h"

#include "libdisplay/libdisplay.h"

LineSegment::LineSegment(const unsigned char color[3], const int thickness,  
                         const double X0, const double Y0,
                         const double X1, const double Y1)
    : Annotation(color), X0_(X0), Y0_(Y0), X1_(X1), Y1_(Y1)
{
    thickness_ = 0.5 * (thickness - 1);
    if (thickness_ < 0) thickness_ = 0;
}

LineSegment::~LineSegment()
{
}

void
LineSegment::Draw(DisplayBase *display)
{
    const double width = display->Width();
    const double height = display->Height();

    // return if the line segment is not on the screen
    if ((X0_ < 0 && X1_ < 0)
        || (X0_ >= width && X1_ >= width)
        || (Y0_ < 0 && Y1_ < 0)
        || (Y0_ >= height && Y1_ >= height)) return;

    // check if this is a vertical line (infinite slope!)
    if (X0_ == X1_)
    {
        double y0 = (Y0_ < Y1_ ? Y0_ : Y1_);
        double y1 = (Y0_ < Y1_ ? Y1_ : Y0_);
        for (double y = y0; y < y1; y++)
        {
            for (double i = 0; i <= thickness_; i += 0.25)
            {
                display->setPixel(X0_ + i, y, color_);
                if (i > 0) display->setPixel(X0_ - i, y, color_);
            }
        }
        return;
    }

    // check if this is a horizontal line
    if (Y0_ == Y1_)
    {
        double x0 = (X0_ < X1_ ? X0_ : X1_);
        double x1 = (X0_ < X1_ ? X1_ : X0_);
        for (double x = x0; x < x1; x++)
        {
            for (double i = 0; i <= thickness_; i += 0.25)
            {
                display->setPixel(x, Y0_ + i, color_);
                if (i > 0) display->setPixel(x, Y0_ - i, color_);
            }
        }
        return;
    }

    const double slope = (Y1_ - Y0_) / (X1_ - X0_);
    const double hypot = sqrt((Y1_ - Y0_) * (Y1_ - Y0_) 
                              + (X1_ - X0_) * (X1_ - X0_));
    const double cosTheta = (X1_ - X0_) / hypot;
    const double sinTheta = (Y1_ - Y0_) / hypot;

    if (fabs(slope) < 1)
    {
        // find the starting point
        double y = Y0_;
        double x0 = X0_;
        double x1 = X1_;
        if (X0_ > X1_)
        {
            y = Y1_;
            x0 = X1_;
            x1 = X0_;
        }

        // check if the point is off of the screen
        if (x0 < 0) 
        {
            y -= slope * x0;
            x0 = 0;
        }
        if (x1 >= width) x1 = width - 1;

        // draw the line
        for (double x = x0; x < x1; x++)
        {
            for (double i = 0; i <= thickness_; i += 0.25)
            {
                display->setPixel(x + i * sinTheta, y - i * cosTheta, color_);
                if (i > 0)
                    display->setPixel(x - i * sinTheta, y + i * cosTheta, 
                                      color_);
            }
            y += slope;
        }
    }
    else
    {
        double x = X0_;
        double y0 = Y0_;
        double y1 = Y1_;
        if (Y0_ > Y1_)
        {
            x = X1_;
            y0 = Y1_;
            y1 = Y0_;
        }
        if (y0 < 0) 
        {
            x -= y0/slope;
            y0 = 0;
        }
        if (y1 >= height) y1 = height - 1;
        for (double y = y0; y < y1; y++)
        {
            for (double i = 0; i <= thickness_; i += 0.25)
            {
                display->setPixel(x + i * sinTheta, y - i * cosTheta, color_);
                if (i > 0)
                    display->setPixel(x - i * sinTheta, y + i * cosTheta, 
                                      color_);
            }
            x += (1/slope);
        }
    }
}
