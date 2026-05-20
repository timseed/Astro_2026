#ifndef PROJECTIONANCIENT_H
#define PROJECTIONANCIENT_H

// This projection contributed by Richard Rognlie <rrognlie@gamerz.net>

#include "ProjectionBase.h"

class ProjectionAncient : public ProjectionBase
{
 public:
    ProjectionAncient(const int f, const int w, const int h);

    bool pixelToSpherical(const double x, const double y, 
			  double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    double dispScale_;
};

#endif
