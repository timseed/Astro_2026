#include <cmath>
#include <map>
using namespace std;

#include "xpUtil.h"

#include "drawArc.h"
#include "drawCircle.h"
#include "libannotate/libannotate.h"
#include "libprojection/ProjectionRectangular.h"

//  Given an value X and radius d, this routine draws a half-circle in
//  the plane where X is constant.
static void
drawAltitudeHalfCirc(ProjectionRectangular *rect, 
                     const double X, const double d, bool Z_positive, 
                     const unsigned char color[3], const int thickness, 
                     const double spacing, const double magnify,
                     Planet *planet, View *view, ProjectionBase *projection,
                     multimap<double, Annotation *> &annotationMap)
{
    double Y = d;
    double Z = (1 - X*X - Y*Y);
    if (fabs(Z) < 1e-5) Z = 0;
    Z = (Z_positive ? sqrt(Z) : -sqrt(Z));

    double lat = M_PI_2 - acos(Z);
    double lon = atan2(Y, X);
    rect->RotateXYZ(lat, lon);

    for (double sinY = 1; sinY >= -1; sinY -= 0.1)
    {
        double prevLat = lat;
        double prevLon = lon;

        Y = sin(M_PI_2 * sinY) * d;
        Z = (1 - X*X - Y*Y);
        if (fabs(Z) < 1e-5) Z = 0;
        Z = (Z_positive ? sqrt(Z) : -sqrt(Z));

        lat = M_PI_2 - acos(Z);
        lon = atan2(Y, X);
        rect->RotateXYZ(lat, lon);

        drawArc(prevLat, prevLon, 1, lat, lon, 1, color, thickness, 
                spacing * deg_to_rad, magnify,
                planet, view, projection, annotationMap);
    }
}

// Draw a circle centered at lat, lon with angular radius rad
void
drawCircle(const double lat, const double lon, const double rad,
           const unsigned char color[3], const int thickness,
           const double spacing, const double magnify,
           Planet *planet, View *view, ProjectionBase *projection,
           multimap<double, Annotation *> &annotationMap)
{
    ProjectionRectangular *rect = new ProjectionRectangular(1, 0, 0);

    rect->SetXYZRotationMatrix(0, lat, -lon);

    drawAltitudeHalfCirc(rect, cos(rad), sin(rad), true,
                         color, thickness, spacing, magnify, planet, view,
                         projection, annotationMap);

    drawAltitudeHalfCirc(rect, cos(rad), sin(rad), false,
                         color, thickness, spacing, magnify, planet, view,
                         projection, annotationMap);

    delete rect;
}
