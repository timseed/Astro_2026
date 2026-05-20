#ifndef PROJECTIONBONNE_H
#define PROJECTIONBONNE_H

#include "ProjectionBase.h"

class ProjectionBonne : public ProjectionBase
{
 public:
    ProjectionBonne(const int f, const int w, const int h);
    bool pixelToSpherical(const double x, const double y, 
			  double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    bool sfl_;

    double lat1_, cotLat1_;
    double Y0_;
    double scale_, yOffset_;
    double sign_;
};

#endif
