//   This projection was contributed by Richard Rognlie <rrognlie@gamerz.net>

#include <cmath>
using namespace std;

#include "ProjectionHemisphere.h"
#include "xpUtil.h"

#define VERTICAL 0

ProjectionHemisphere::ProjectionHemisphere(const int f, const int w, 
                                           const int h) 
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = false;

#if VERTICAL
    if (height_/2 < width_)
        dispScale_ = height_/2;
    else
        dispScale_ = width_;
#else
    if (width_/2 < height_)
        dispScale_ = width_/2;
    else
        dispScale_ = height_;
#endif

    radius_ /= 2;
    dispScale_ *= 2;

    buildPhotoTable();
}

ProjectionHemisphere::~ProjectionHemisphere() 
{
    destroyPhotoTable();
}

bool
ProjectionHemisphere::pixelToSpherical(const double x, const double y, 
                                       double &lon, double &lat)
{
    double X = (x - centerX_) / dispScale_;
    double Y = - (y - centerY_) / dispScale_;

#if VERTICAL
    if (Y<0) 
    {
        Y += 0.25;
        double arg = Y/radius_;
        if (fabs(arg) > 1) return(false);
        lat = asin(arg);

        arg = -X / (radius_ * cos(lat));
        if (fabs(arg) > 1) return(false);
        lon = acos(arg) - M_PI;
    }
    else 
    {
        Y -= 0.25;
        double arg = Y/radius_;
        if (fabs(arg) > 1) return(false);
        lat = asin(arg);

        arg = -X / (radius_ * cos(lat));
        if (fabs(arg) > 1) return(false);
        lon = acos(arg);
    }
#else
    double arg = Y / radius_;
    if (fabs(arg) > 1) return(false);
    lat = asin(arg);

    if (X<0) 
    {
        X += 0.25;
        arg = -X / (radius_ * cos(lat));
        if (fabs(arg) > 1) return(false);
        lon = acos(arg) - M_PI;
    }
    else 
    {
        X -= 0.25;
        arg = -X / (radius_ * cos(lat));
        if (fabs(arg) > 1) return(false);
        lon = acos(arg);
    }
#endif

    lon -= M_PI/2; 

    darkening_ = getPhotoFunction(fabs(cos(lon) * cos(lat)));

    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionHemisphere::sphericalToPixel(double lon, double lat, 
                                       double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);

    lon += M_PI/2;

    if (lon > M_PI) lon -= TWO_PI;

    double Y = radius_ * sin(lat);
    double X;

#if VERTICAL
    if (lon < 0) 
    {
        Y -= 0.25;
        X = radius_ * cos(lat) * sin(lon-3*M_PI/2);
    }
    else 
    {
        Y += 0.25;
        X = radius_ * cos(lat) * sin(lon-M_PI/2);
    }
#else
    if (lon < 0) 
    {
        X = radius_ * cos(lat) * sin(lon-3*M_PI/2) - 0.25;
    }
    else 
    {
        X = radius_ * cos(lat) * sin(lon-M_PI/2) + 0.25;
    }
#endif

    x = centerX_ + dispScale_ * X;
    if (x < 0 || x >= width_) return(false);

    y = centerY_ - dispScale_ * Y;
    if (y < 0 || y >= height_) return(false);

    return(true);
}
