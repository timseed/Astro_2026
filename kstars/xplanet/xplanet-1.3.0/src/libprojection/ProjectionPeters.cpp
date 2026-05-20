// This projection contributed by Martin Pool <mbp@linuxcare.com> 

#include <cmath>
using namespace std;

#include "ProjectionPeters.h"
#include "xpUtil.h"

/*
 * This class implements the Peters projection, which is an
 * area-preserving variant of the Mercator projection.  I'm not 100%
 * sure this is the authentic Peters algorithm, but it seems to have
 * the right properties.
 *
 * Lines of latitude run straight across the map, and area is more or
 * less preserved.  So, lines of latitude squish together towards the
 * poles, and spread out around the Equator.
 *
 * Longitude maps directly to X (with a scaling factor).
 *
 * Y is proportional to the sin of latitude, adjusted so that 0 is in
 * the center and +1/-1 at the edges.
 *
 * This projection always preserves the aspect ratio, since it already
 * tends to distort the vertical scale.  So if it's not displayed in a
 * 2:1 window there will be black space.
 *
 * This projection cannot rotate vertically because I can't work out
 * what it should look like in that case.  Sorry.
 */

ProjectionPeters::ProjectionPeters(const int f, const int w, const int h) 
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = true;
    wd_ = static_cast<int> (2 * width_ * radius_);
    ht_ = wd_/2;
}

bool
ProjectionPeters::pixelToSpherical(const double x, const double y,
                                   double &lon, double &lat)
{
    const double X = ((x - width_/2)) * TWO_PI / wd_;
    const double Y = ((height_ - 2*y)) / ht_;

    if (Y < -1 || Y > +1) return(false);

    lat = asin(Y);
    lon = X;
  
    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionPeters::sphericalToPixel(double lon, double lat, 
                                   double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);

    double X = lon;
    const double Y = sin(lat);

    if (X >= M_PI) X -= TWO_PI;
    else if (X < -M_PI) X += TWO_PI;

    x = (wd_ * X / TWO_PI) + width_ / 2;
    y = height_ / 2 - (Y * ht_/2);

    if (x < 0 || x >= width_) return(false);
    if (y < 0 || y >= height_) return(false);

    return(true);
}
