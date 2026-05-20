#include <cmath>
using namespace std;

#include "Options.h"
#include "libdisplay/libdisplay.h"

void
drawSunGlare(DisplayBase *display, const double X, const double Y, 
             const double R, const unsigned char *color)
{
    Options *options = Options::getInstance();
    const double glare = options->Glare() * R;

    if (glare == 0) return;

    const double falloff = options->Glare() / log(256.0);

    const int height = display->Height();
    const int width = display->Width();
    for (int j = 0; j < height; j++)
    {
        const double jdist = Y - j;      // decreases downward
        for (int i = 0; i < width; i++)
        {
            const double idist = i - X;  // increases to the right
            const double dist = sqrt(idist * idist + jdist * jdist);
            if (dist > R-3 && dist < glare)
            {
                // draw the glare
                const double angle = atan2(jdist, idist);
                const double brightness = (0.05 * (19 + cos(12 * angle)) 
                                           * exp((1-dist/R)/falloff));
                
                display->setPixel(i, j, color, brightness);
            }
        }
    }
}
