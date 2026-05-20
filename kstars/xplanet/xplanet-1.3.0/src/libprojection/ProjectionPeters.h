#ifndef PROJECTIONPETERS_H
#define PROJECTIONPETERS_H

// This projection contributed by Martin Pool <mbp@linuxcare.com> 

#include "ProjectionBase.h"

class ProjectionPeters : public ProjectionBase
{
 public:
    ProjectionPeters(const int f, const int w, const int h);
    bool pixelToSpherical(const double x, const double y, 
			  double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;
    
 private:
    int wd_;
    int ht_;
};

#endif
