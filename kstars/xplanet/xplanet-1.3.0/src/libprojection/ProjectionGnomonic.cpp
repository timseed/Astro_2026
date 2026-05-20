/*
  This projection is from Section 5.5.1 of Calabretta and Greisen
  (2002), "Representations of celestial coordinates in FITS",
  Astronomy and Astrophysics 395, 1077-1122.

  The paper is available online at
  http://www.atnf.csiro.au/~mcalabre/WCS
*/

#include <cmath>
#include <cstdio>
#include <sstream>
using namespace std;

#include "Options.h"
#include "ProjectionGnomonic.h"
#include "xpUtil.h"

ProjectionGnomonic::ProjectionGnomonic(const int f, const int w, const int h) 
    : ProjectionBase(f, w, h) 
{
    init(f, w, h, Options::getInstance());
}

ProjectionGnomonic::ProjectionGnomonic(const int f, const int w, const int h,
                                       const Options* o) 
    : ProjectionBase(f, w, h, o) 
{
    init(f, w, h, o);
}

void ProjectionGnomonic::init(const int f, const int w, const int h,
                              const Options* options) 
{
    isWrapAround_ = false;

    double lat1 = 45 * deg_to_rad;

    vector<double> projParams = options->ProjectionParameters();
    if (!projParams.empty())
    {
        if (fabs(projParams[0]) > 0 && fabs(projParams[0]) < M_PI_2)
        {
            lat1 = projParams[0];
        }
        else
        {
            char buffer[256];
            snprintf(buffer, 256, "%.1f", projParams[0]/deg_to_rad);

            ostringstream errMsg;
            errMsg << "Projection latitude of " << buffer
                   << " degrees is out of range for Gnomonic Projection.";
            snprintf(buffer, 256, "  Using %.1f degrees.\n", lat1/deg_to_rad);
            errMsg << buffer;
            xpWarn(errMsg.str(), __FILE__, __LINE__);
        }
    }

    scale_ = 1/tan(lat1);
}

bool
ProjectionGnomonic::pixelToSpherical(const double x, const double y, 
                                     double &lon, double &lat)
{
    const double offsetX = x + width_/2 - centerX_;
    const double offsetY = y + height_/2 - centerY_;

    const double X = (offsetX/width_ - 0.5);
    const double Y = (0.5 - offsetY/height_);

    lon = atan(2*X/scale_);
    lat = atan(2*Y/scale_ * cos(lon));

    if (fabs(lon) > M_PI) return(false);

    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionGnomonic::sphericalToPixel(double lon, double lat,
                                     double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);

    if (fabs(lon) > M_PI_2) return(false);

    // X and Y range from -scale/2 to scale/2
    double X = 0.5 * scale_ * tan(lon);
    double Y = 0.5 * scale_ * tan(lat) / cos(lon);

    x = width_ * (X + 0.5);
    y = height_ * (0.5 - Y);

    x += (centerX_ - width_/2);
    y += (centerY_ - height_/2);

    if (y < 0 || y >= height_) return(false);

    return(true);
}
