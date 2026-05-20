#include <cmath>
#include <map>
using namespace std;

#include "sphericalToPixel.h"
#include "xpUtil.h"

#include "drawArc.h"
#include "libannotate/libannotate.h"

void
drawArc(const double lat1, const double lon1, const double rad1,
        const double lat2, const double lon2, const double rad2,
        const unsigned char color[3], const int thickness, 
        const double spacing, const double magnify,
        Planet *planet, View *view, ProjectionBase *projection,
        multimap<double, Annotation *> &annotationMap)
{
    double tc, dist;
    calcGreatArc(lat1, lon1, lat2, lon2, tc, dist);

    const double sin_lat1 = sin(lat1);
    const double cos_lat1 = cos(lat1);
    const double sin_tc = sin(tc);
    const double cos_tc = cos(tc);

    double prevX = 0, prevY = 0, prevZ = 0;
    double prevLength2 = 0;
    bool drawPrev;
    bool firstTime = true;

    for (double d = 0; d < dist; d += spacing)
    {
        const double lat = asin(sin_lat1 * cos(d) 
                                + cos_lat1 * sin(d) * cos_tc);
        const double dlon = atan2(sin_tc * sin(d) * cos_lat1, 
                                  cos(d) - sin_lat1 * sin(lat));
        const double lon = fmod(lon1 - dlon + M_PI, TWO_PI) - M_PI;
        const double rad = rad1 + (rad2 - rad1) * d/dist;
        
        double X, Y, Z;
        const bool drawThis = sphericalToPixel(lat, lon, rad * magnify, 
                                               X, Y, Z, 
                                               planet, view, projection);

        if (!firstTime)
        {
            double X1, Y1, Z1;
            double X2, Y2, Z2;

            if (prevZ > Z)
            {
                X1 = X;
                Y1 = Y;
                Z1 = Z;
                X2 = prevX;
                Y2 = prevY;
                Z2 = prevZ;
            }
            else
            {
                X1 = prevX;
                Y1 = prevY;
                Z1 = prevZ;
                X2 = X;
                Y2 = Y;
                Z2 = Z;
            }
            
            if (drawThis && drawPrev)
            {
                const double length2 = ((X2-X1) * (X2-X1) 
                                        + (Y2-Y1) * (Y2-Y1));
                if (length2 < 10*prevLength2)
                {
                    LineSegment *ls = new LineSegment(color, thickness, 
                                                      X2, Y2, X1, Y1);
                    annotationMap.insert(pair<const double, Annotation*>(Z, ls));
                }
                prevLength2 = length2;
            }
        }
        prevX = X;
        prevY = Y;
        prevZ = Z;

        drawPrev = drawThis;

        firstTime = false;
    }
}
