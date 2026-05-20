#include <cmath>
#include <cstdio>
#include <sstream>
using namespace std;

#include "Options.h"
#include "ProjectionMercator.h"
#include "xpUtil.h"

ProjectionMercator::ProjectionMercator(const int f, const int w, const int h) 
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = true;

    double lat1 = 80 * deg_to_rad;
    Options *options = Options::getInstance();
    vector<double> projParams = options->ProjectionParameters();
    if (!projParams.empty())
    {
        if (fabs(projParams[0]) > 0 && fabs(projParams[0]) < M_PI_2)
        {
            lat1 = fabs(projParams[0]);
        }
        else
        {
            char buffer[256];
            snprintf(buffer, 256, "%.1f", projParams[0]/deg_to_rad);

            ostringstream errMsg;
            errMsg << "Projection latitude of " << buffer
                   << " degrees is out of range for Mercator Projection.";
            snprintf(buffer, 256, "  Using %.1f degrees.\n", lat1/deg_to_rad);
            errMsg << buffer;
            xpWarn(errMsg.str(), __FILE__, __LINE__);
        }
    }

    yScale_ = log(tan(M_PI/4 + lat1/2))/M_PI_2;
}

bool
ProjectionMercator::pixelToSpherical(const double x, const double y, 
                                     double &lon, double &lat)
{
    const double X = ((x - width_/2)) * TWO_PI / width_;
    const double Y = ((height_/2 - y)) * yScale_ * M_PI / height_;

    lat = atan(sinh(Y));
    lon = X;
  
    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionMercator::sphericalToPixel(double lon, double lat,
                                     double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);
  
    double X = lon;
    const double Y = log(tan(M_PI/4 + lat/2));

    if (X >= M_PI) X -= TWO_PI;
    else if (X < -M_PI) X += TWO_PI;

    x = ((width_ * X / TWO_PI) + width_ / 2);
    y = (height_ / 2 - (height_ * Y / (yScale_ * M_PI)));
  
    if (y < 0 || y >= height_) return(false);
    return(true);
}
