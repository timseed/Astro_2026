#include <cmath>
using namespace std;

#include "ProjectionMollweide.h"
#include "xpUtil.h"

ProjectionMollweide::ProjectionMollweide(const int f, const int w, const int h) 
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = false;
    // radius is M_SQRT2 * R from Snyder (1987), p 251
}

bool
ProjectionMollweide::pixelToSpherical(const double x, const double y,
                                      double &lon, double &lat)
{
    const double X = 2.0 * (x + width_/2 - centerX_) / width_ - 1;
    const double Y = 1 - 2.0 * (y + height_/2 - centerY_) / height_;

    double arg = Y / radius_;
    if (fabs(arg) > 1) return(false);
    const double theta = asin(arg);

    arg = (2 * theta + sin (2*theta)) / M_PI;
    if (fabs (arg) > 1) return(false);
    lat = asin(arg);

    if (fabs(theta) == M_PI)
        lon = 0;
    else 
    {
        lon = M_PI * X / (2 * radius_ * cos(theta));
        if (fabs(lon) > M_PI) return(false);
    }
 
    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionMollweide::sphericalToPixel(double lon, double lat,
                                      double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);

    double theta = lat;
    double del_theta = 1;
    while (fabs(del_theta) > 1e-5)
    {
        del_theta = -((theta + sin(theta) - M_PI * sin(lat))
                      / (1 + cos(theta)));
        theta += del_theta;
    }
    theta /= 2;

    while (lon < -M_PI) lon += TWO_PI;
    while (lon > M_PI) lon -= TWO_PI;

    const double X = (2 * radius_ / M_PI) * lon * cos(theta);
    const double Y = radius_ * sin(theta);

    x = (X + 1) * width_/2 + centerX_ - width_/2;
    if (x < 0 || x >= width_) return(false);

    y = height_/2 * (1 - Y) + centerY_ - height_/2;
    if (y < 0 || y >= height_) return(false);

    return(true);
}
