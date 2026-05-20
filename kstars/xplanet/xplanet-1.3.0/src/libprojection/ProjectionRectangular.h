#ifndef PROJECTIONRECTANGULAR_H
#define PROJECTIONRECTANGULAR_H

#include "ProjectionBase.h"

class ProjectionRectangular : public ProjectionBase
{
 public:
    ProjectionRectangular(const int f, const int w, const int h);
    ProjectionRectangular(const int f, const int w, 
			  const int h,
			  const double startLat,
			  const double startLon,
			  const double mapHeight,
			  const double mapWidth);

    bool pixelToSpherical(const double x, const double y, 
			  double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    bool mapBounds_;

    double startLat_, startLon_;
    double delLat_, delLon_;
};

#endif
