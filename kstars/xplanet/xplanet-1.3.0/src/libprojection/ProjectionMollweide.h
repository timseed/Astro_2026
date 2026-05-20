#ifndef PROJECTIONMOLLWEIDE_H
#define PROJECTIONMOLLWEIDE_H

#include "ProjectionBase.h"

class ProjectionMollweide : public ProjectionBase
{
 public:
    ProjectionMollweide(const int f, const int w, const int h);
    bool pixelToSpherical(const double x, const double y, 
			  double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;
};

#endif
