// This projection was contributed by Richard Rognlie <rrognlie@gamerz.net>

#include <cmath>
using namespace std;

#include "ProjectionAncient.h"
#include "xpUtil.h"

ProjectionAncient::ProjectionAncient(const int f, const int w, const int h)
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = false;

    if (width_/2 < height_)
        dispScale_ = width_/2;
    else
        dispScale_ = height_;
}

bool
ProjectionAncient::pixelToSpherical(const double x, const double y,
                                    double &lon, double &lat)
{
    double X = (x - centerX_) / dispScale_;
    double Y = - (y - centerY_) / dispScale_;
    double rho,c;

    if (X<0) {
        X += radius_;
        rho = sqrt(X*X + Y*Y);
        if (rho > radius_) return(0);

        c = M_PI/2 * rho / radius_;

        if (rho == 0) 
        {
            lat = 0;
            lon = 0;
        }
        else 
        {       
            double arg = Y * sin(c) / rho;
            lat = asin(arg);
            lon = atan2(X * sin(c), rho * cos(c)) - M_PI/2;
        }
    }
    else 
    {
        X -= radius_;
        rho = sqrt(X*X + Y*Y);
        if (rho > radius_) return(0);

        c = M_PI/2 * rho / radius_;

        if (rho == 0) 
        {
            lat = 0;
            lon = 0;
        }
        else 
        {       
            double arg = Y * sin(c) / rho;
            lat = asin(arg);
            lon = atan2(X * sin(c), rho * cos(c)) + M_PI/2;
        }
    }

    lon -= M_PI/2; 

    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionAncient::sphericalToPixel(double lon, double lat, 
                                    double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);

    lon += M_PI/2;

    if (lon > M_PI) lon -= TWO_PI;

    double c,X,Y;
    double k = 2*radius_ / M_PI;

    if (lon < 0) 
    {
        lon += M_PI/2;
        c = acos(cos(lat) * cos(lon));
        if (c != 0)
            k *= c / sin(c);
        X = -radius_ + k * cos(lat) * sin(lon);
        Y = k * sin(lat);
    }
    else 
    {
        lon -= M_PI/2;
        c = acos(cos(lat) * cos(lon));
        if (c != 0)
            k *= c / sin(c);
        X = radius_ + k * cos(lat) * sin(lon);
        Y = k * sin(lat);
    }

    x = centerX_ + dispScale_ * X;
    if (x < 0 || x >= width_) return(false);

    y = centerY_ - dispScale_ * Y;
    if (y < 0 || y >= height_) return(false);

    return(true);
}
