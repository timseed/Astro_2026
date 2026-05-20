#ifndef PROJECTIONMERCATOR_H
#define PROJECTIONMERCATOR_H

#include "ProjectionBase.h"

class ProjectionMercator : public ProjectionBase
{
 public:
    ProjectionMercator(const int f, const int w, const int h);
    bool pixelToSpherical(const double x, const double y, 
			  double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    double yScale_;
};

#endif
