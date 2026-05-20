#include <cmath>
using namespace std;

#include "ProjectionAzimuthal.h"
#include "xpUtil.h"

ProjectionAzimuthal::ProjectionAzimuthal(const int f, const int w, 
                                         const int h) 
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = false;
    radius_ = sqrt(2 * radius_);

    // The rendered globe is contained in a square with sides of length
    // iside and upper left corner at (istart, jstart).
    iside_ = (int) (radius_ * height_);
}

bool
ProjectionAzimuthal::pixelToSpherical(const double x, const double y, 
                                      double &lon, double &lat)
{
    const double X = 2.0 * (x - centerX_) / iside_;
    const double Y = -2.0 * (y - centerY_) / iside_;

    const double rho = sqrt(X*X + Y*Y);
    if (rho > radius_) return(false);

    const double c = M_PI * rho / radius_;

    if (rho == 0) 
    {
        lat = 0;
        lon = 0;
    }
    else 
    {
        const double arg = Y * sin(c) / rho;
        lat = asin(arg);
        lon = atan2 (X * sin(c), rho * cos(c));
    }

    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionAzimuthal::sphericalToPixel(double lon, double lat,
                                      double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);
    
    const double c = acos(cos(lat) * cos(lon));
    if (c == M_PI) return(false);

    double k = radius_ / M_PI;
    if (c != 0)
        k *= c / sin(c);
    
    const double X = k * cos(lat) * sin(lon);
    const double Y = k * sin(lat);

    x = centerX_ + X * iside_/2;
    if (x < 0 || x >= width_) return(false);

    y = centerY_ - Y * iside_/2;
    if (y < 0 || y >= height_) return(false);

    return(true);
}

