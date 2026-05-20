/*
  This projection is from Section 5.5.2 of Calabretta and Greisen
  (2002), "Representations of celestial coordinates in FITS",
  Astronomy and Astrophysics 395, 1077-1122.

  The paper is available online at
  http://www.atnf.csiro.au/~mcalabre/WCS
 */

#include <cmath>
using namespace std;

#include "ProjectionPolyconic.h"
#include "xpUtil.h"

ProjectionPolyconic::ProjectionPolyconic(const int f, const int w, const int h)
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = false;

    // find the highest point of the shape and scale the image so that
    // it fits.
    double topValue = 0;
    const double lon = M_PI;
    for (int i = 0; i < height_; i++)
    {
        double lat = i * M_PI / (height_ - 1) - M_PI_2;
        if (lat == 0) continue;

        const double E = lon * sin(lat);
        double Y = lat + (1 - cos(E)) / tan(lat);
        Y = height_ * (0.5 - Y/M_PI);
        if (Y < topValue) topValue = Y;
    }

    // topValue will be <= 0
    scale_ = height_ / (height_ - 2*topValue);

    scale_ *= (2*radius_);
}

double
ProjectionPolyconic::F(const double X, const double Y, const double theta)
{
    double YmT = Y - theta;
    double returnVal = X*X + YmT * (YmT - 2 / tan(theta));
    return(returnVal);
}

double
ProjectionPolyconic::Fprime(const double X, const double Y, const double theta)
{
    double tanTheta = tan(theta);
    double returnVal = 2 * (1 + (Y-theta)/tanTheta) / tanTheta;
    return(returnVal);
}

double
ProjectionPolyconic::rootF(const double X, const double Y)
{
    double delTheta = 0.001;
    if (fabs(Y) < delTheta) return(0);

    double returnVal;

#if 1
    // Newton's method
    double lat;
    if (F(X, Y, delTheta) * F(X, Y, M_PI_2) < 0)
    {
        lat = delTheta;
    }
    else if (F(X, Y, -delTheta) * F(X, Y, -M_PI_2) < 0)
    {
        lat = -delTheta;
    }
    else
    {
        return(0);
    }
    
    double del_lat = 1;
    while (fabs(del_lat) > 1e-5)
    {
        double Fp = Fprime(X, Y, lat);
        del_lat = -F(X, Y, lat) / Fp;
        lat += del_lat;
        if (fabs(lat) > M_PI_2) return(M_PI_2);
    }

    returnVal = lat;
#else
    // bisection method
    double lim0, lim1;
    if (F(X, Y, delTheta) * F(X, Y, M_PI_2) < 0)
    {
        lim0 = delTheta;
        lim1 = M_PI_2;
    }
    else if (F(X, Y, -delTheta) * F(X, Y, -M_PI_2) < 0)
    {
        lim0 = -M_PI_2;
        lim1 = -delTheta;
    }
    else
    {
        return(0);
    }

    while (fabs(lim0 - lim1) > 1e-5)
    {
        double mid = 0.5 * (lim0 + lim1);
        double val = F(X, Y, mid);
        
        if (F(X, Y, lim0) * val < 0) 
        {
            lim1 = mid;
        }
        else if (F(X, Y, lim1) * val < 0) 
        {
            lim0 = mid;
        }
        else 
        {
            return(0);
        }
    }
    returnVal = lim0;
#endif

    return(returnVal);
}

bool
ProjectionPolyconic::pixelToSpherical(const double x, const double y,
                                      double &lon, double &lat)
{
    const double offsetX = x + width_/2 - centerX_;
    const double offsetY = y + height_/2 - centerY_;

    const double X = TWO_PI * (offsetX/width_ - 0.5)/scale_;
    const double Y = M_PI * (0.5 - offsetY/height_)/scale_;

    lat = rootF(X, Y);
    if (fabs(lat) > M_PI_2) return(false);
    if (sin(lat) == 0)
    {
        lon = X;
    }
    else
    {
        double tanLat = tan(lat);
        lon = atan2(X * tanLat, 1 - (Y-lat) * tanLat) / sin(lat);
    }

    if (fabs(lon) > M_PI) return(false);
 
    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionPolyconic::sphericalToPixel(double lon, double lat,
                                      double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);

    double X, Y;

    const double tanLat = tan(lat);
    if (tanLat == 0)
    {
        X = lon;
        Y = 0;
    }
    else
    {
        const double E = lon * sin(lat);
        X = sin(E) / tanLat;
        Y = lat + (1 - cos(E)) / tanLat;
    }

    x = width_ * (X * scale_ / TWO_PI + 0.5);
    y = height_ * (0.5 - Y * scale_ / M_PI);

    x += (centerX_ - width_/2);
    y += (centerY_ - height_/2);

    if (y < 0 || y >= height_) return(false);

    return(true);
}
