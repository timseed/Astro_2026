#ifndef PROJECTIONORTHOGRAPHIC_H
#define PROJECTIONORTHOGRAPHIC_H

#include "ProjectionBase.h"

class ProjectionOrthographic : public ProjectionBase
{
 public:
    ProjectionOrthographic(const int f, const int w, const int h);
    ~ProjectionOrthographic();

    void setRange(const double range);

    bool pixelToSpherical(const double x, const double y, 
			  double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    double P, Psq, Pm1, Pp1, PPm1, Pm1sq;
    double dispScale_;
};

#endif
