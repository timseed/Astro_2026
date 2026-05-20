#include <cmath>
using namespace std;

#include "ProjectionRectangular.h"
#include "xpUtil.h"

ProjectionRectangular::ProjectionRectangular(const int f, const int w,
                                             const int h) 
    : ProjectionBase (f, w, h),
      mapBounds_(false)
{
    isWrapAround_ = true;

    startLon_ = -M_PI + centerLon_;
    startLat_ = M_PI_2;

    delLat_ = M_PI/height_;
    delLon_ = TWO_PI/width_;
}

ProjectionRectangular::ProjectionRectangular(const int f, const int w, 
                                             const int h,
                                             const double startLat,
                                             const double startLon,
                                             const double mapHeight,
                                             const double mapWidth)
    : ProjectionBase (f, w, h), mapBounds_(true)
{
    startLon_ = startLon * f;
    startLat_ = startLat;

    delLat_ = mapHeight/height_;
    delLon_ = mapWidth/width_ * f;
}

bool
ProjectionRectangular::pixelToSpherical(const double x, const double y, 
                                        double &lon, double &lat)
{
    lon = x * delLon_ + startLon_;
    lat = startLat_ - y * delLat_;
    return(true);
}

bool
ProjectionRectangular::sphericalToPixel(double lon, double lat,
                                        double &x, double &y) const
{
    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;
          
    x = (lon - startLon_)/delLon_;

    if (!mapBounds_)
    {
        if (x >= width_) 
            x -= width_;
        else if (x < 0) 
            x += width_;
    }

    y = (startLat_ - lat)/delLat_;

    if (!mapBounds_ && y >= height_) y = height_ - 1;
    
    return(true);
}

