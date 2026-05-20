#include <cmath>
using namespace std;

#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /* pi/2 */
#endif

#include "Symbol.h"

#include "libdisplay/libdisplay.h"

Symbol::Symbol(const unsigned char color[3], 
               const int x, const int y, const int r)
    : Annotation(color), x_(x), y_(y), r_(r)
{
    width_ = 2*r;
    height_ = 2*r;
}

Symbol::~Symbol()
{
}

void
Symbol::DrawCircle(DisplayBase *display, const int r, 
                   const unsigned char color[3])
{
    if (r <= 0) return;

    int xx, yy;
    double r2 = r * r;
    double dd = 1 / (M_PI_2 * r);
    for (double d = 0; d < M_PI_2; d += dd)
    {
        xx = static_cast<int>(cos(d) * r + 0.5);
        yy = static_cast<int>(sin(d) * r + 0.5);
        double opacity = (xx * xx + yy * yy) / r2;
        if (opacity > 1) opacity = 1/opacity;

        display->setPixel(x_ + xx, y_ + yy, color, opacity);
        display->setPixel(x_ - xx, y_ + yy, color, opacity);
        display->setPixel(x_ + xx, y_ - yy, color, opacity);
        display->setPixel(x_ - xx, y_ - yy, color, opacity);
    }
    display->setPixel(x_, y_ + r, color);
    display->setPixel(x_, y_ - r, color);
}

void
Symbol::Draw(DisplayBase *display)
{
    if (r_ < 1) return;
    unsigned char black[3] = { 0, 0, 0 };
    DrawCircle(display, r_ - 1, black);
    DrawCircle(display, r_ + 1, black);

    DrawCircle(display, r_, color_);
}

