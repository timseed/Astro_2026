#ifndef PROJECTIONAZIMUTHAL_H
#define PROJECTIONAZIMUTHAL_H

#include "ProjectionBase.h"

class ProjectionAzimuthal : public ProjectionBase
{
 public:
    ProjectionAzimuthal(const int f, const int w, const int h);
    bool pixelToSpherical(const double x, const double y, 
			  double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    int iside_;
};

#endif
