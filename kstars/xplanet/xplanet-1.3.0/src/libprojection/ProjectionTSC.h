#ifndef PROJECTIONTSC_H
#define PROJECTIONTSC_H

#include "ProjectionBase.h"

class ProjectionTSC : public ProjectionBase
{
 public:
    ProjectionTSC(const int f, const int w, const int h);
    bool pixelToSpherical(const double x, const double y, 
                          double &lon, double &lat);

    bool sphericalToPixel(double lon, double lat, double &x, double &y) const;

 private:
    int trueWidth_;
    int trueHeight_;

    double xOffset_;
    double yOffset_;
    
    double xScale_;
    double yScale_;

    double xPixel_[6];
    double yPixel_[6];

    void GetCenterLatLon(const int face, 
                         double &lat_c, double &lon_c) const;

    void GetXiEtaZeta(const int face, 
                      const double l, const double m, const double n,
                      double &xi, double &eta, double &zeta) const;

    int GetFace(const double x, const double y) const;

    void GetLMN(const int face, 
                const double xi, const double eta, const double zeta,
                double &l, double &m, double &n) const;
    
};

#endif
