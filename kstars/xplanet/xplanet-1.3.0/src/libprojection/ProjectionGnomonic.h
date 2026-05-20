#ifndef PROJECTIONGNOMONIC_H
#define PROJECTIONGNOMONIC_H

#include "ProjectionBase.h"

class ProjectionGnomonic : public ProjectionBase
{
 public:
    ProjectionGnomonic(const int f, const int w, const int h);
    ProjectionGnomonic(const int f, const int w, const int h,
                       const Options* o);

    bool pixelToSpherical(const double x, const double y, 
                          double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    void init(const int f, const int w, const int h, const Options* options);

    double scale_;
};

#endif
