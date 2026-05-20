#include <cmath>
using namespace std;

#include "ProjectionLambert.h"
#include "xpUtil.h"

ProjectionLambert::ProjectionLambert(const int f, const int w, const int h) 
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = true;
}

bool
ProjectionLambert::pixelToSpherical(const double x, const double y, 
                                     double &lon, double &lat)
{
    const double X = (x - width_/2) * TWO_PI / width_;
    const double Y = 1 - 2 * y / height_;

    if (fabs(Y) > 1)
    {
        if (Y < 0) 
            lat = -M_PI_2;
        else
            lat = M_PI_2;
    }
    else
    {
        lat = asin(Y);
    }
    lon = X;
  
    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionLambert::sphericalToPixel(double lon, double lat,
                                    double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);
  
    double X = lon;
    const double Y = sin(lat);

    if (X >= M_PI) X -= TWO_PI;
    else if (X < -M_PI) X += TWO_PI;

    x = (width_ * X / TWO_PI) + width_ / 2;
    y = (1 - Y) * height_ / 2;
  
    if (y < 0 || y >= height_) return(false);
    return(true);
}
