#ifndef PROJECTIONLAMBERT_H
#define PROJECTIONLAMBERT_H

#include "ProjectionBase.h"

class ProjectionLambert : public ProjectionBase
{
 public:
    ProjectionLambert(const int f, const int w, const int h);
    bool pixelToSpherical(const double x, const double y, 
                          double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;
};

#endif
