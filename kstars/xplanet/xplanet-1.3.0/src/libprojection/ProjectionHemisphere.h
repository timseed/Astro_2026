#ifndef PROJECTIONHEMISPHERE_H
#define PROJECTIONHEMISPHERE_H

//   This projection was contributed by Richard Rognlie <rrognlie@gamerz.net>

#include "ProjectionBase.h"

class ProjectionHemisphere : public ProjectionBase
{
 public:
    ProjectionHemisphere(const int f, const int w, const int h);
    ~ProjectionHemisphere();

    bool pixelToSpherical(const double x, const double y, 
			  double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    double dispScale_;
};

#endif
