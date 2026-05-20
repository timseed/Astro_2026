#ifndef PROJECTIONPOLYCONIC_H
#define PROJECTIONPOLYCONIC_H

#include "ProjectionBase.h"

class ProjectionPolyconic : public ProjectionBase
{
 public:
    ProjectionPolyconic(const int f, const int w, const int h);
    bool pixelToSpherical(const double x, const double y, 
                          double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    double scale_;

    // Used to iteratively find the latitude given pixel coordinates
    double F(const double X, const double Y, const double theta);
    double Fprime(const double X, const double Y, const double theta);
    double rootF(const double X, const double Y);
};

#endif
