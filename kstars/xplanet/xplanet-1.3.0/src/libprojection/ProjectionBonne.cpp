/*
  This projection is from Section 5.5.1 of Calabretta and Greisen
  (2002), "Representations of celestial coordinates in FITS",
  Astronomy and Astrophysics 395, 1077-1122.

  The paper is available online at
  http://www.atnf.csiro.au/~mcalabre/WCS
 */

#include <cmath>
#include <cstdio>
#include <sstream>
#include <vector>
using namespace std;

#include "Options.h"
#include "ProjectionBonne.h"
#include "xpUtil.h"

ProjectionBonne::ProjectionBonne(const int f, const int w, const int h) 
    : ProjectionBase(f, w, h) 
{
    isWrapAround_ = false;

    double lat1_ = 50 * deg_to_rad;
    
    Options *options = Options::getInstance();
    vector<double> projParams = options->ProjectionParameters();
    if (!projParams.empty())
    {
        if (fabs(projParams[0]) < M_PI_2)
        {
            lat1_ = projParams[0];
        }
        else
        {
            char buffer[256];
            snprintf(buffer, 256, "%.1f", projParams[0]/deg_to_rad);

            ostringstream errMsg;
            errMsg << "Projection latitude of " << buffer
                   << " degrees is out of range for Bonne Projection.  Using ";
            snprintf(buffer, 256, "%.1f degrees.\n", lat1_/deg_to_rad);
            errMsg << buffer;
            xpWarn(errMsg.str(), __FILE__, __LINE__);
        }
    }

    // if lat1_ = 0, this is the Sanson-Flamsteed projection
    sfl_ = (tan(lat1_) == 0);

    if (sfl_)
    {
        Y0_ = 0;
        scale_ = 1;
        yOffset_ = 0;
    }
    else
    {
        const double cotLat1_ = 1/tan(lat1_);
        
        Y0_ = cotLat1_ + lat1_;
        sign_ = lat1_/fabs(lat1_);
        
        // scale the image so that it fits.
        double topValue = 0;
        double bottomValue = height_;
        const double lon = M_PI;
        for (int i = 0; i < height_; i++)
        {
            double lat = i * M_PI / (height_ - 1) - M_PI_2;
            double R = Y0_ - lat;
            double A = lon * cos(lat) / R;
            double Y = height_ * (0.5 - (Y0_ - R * cos(A))/M_PI);
            if (Y < topValue) topValue = Y;
            if (Y > bottomValue) bottomValue = Y;
        }
        
        scale_ = height_ / (bottomValue - topValue);
        yOffset_ = 0.5 * (scale_ * (height_ - bottomValue - topValue));
    }

    scale_ *= (2*radius_);
}

bool
ProjectionBonne::pixelToSpherical(const double x, const double y, 
                                  double &lon, double &lat)
{
    const double offsetX = x + width_/2 - centerX_;
    const double offsetY = y + height_/2 - centerY_ - yOffset_;

    const double X = TWO_PI * (offsetX/width_ - 0.5)/scale_;
    const double Y = Y0_ - M_PI * (0.5 - offsetY/height_)/scale_;

    if (sfl_)
    {
        if (fabs(Y) > M_PI_2) return(false);
        lat = -Y;
        if (cos(lat) == 0)
            lon = 0;
        else
            lon = X/cos(lat);
    }
    else
    {
        const double Rtheta = sign_ * sqrt(X*X + Y*Y);
        
        lat = Y0_ - Rtheta;
        if (fabs(lat) > M_PI_2) return(false);
        
        const double Aphi = atan2(X/Rtheta, Y/Rtheta);
        if (cos(lat) == 0)
            lon = 0;
        else
            lon = Aphi * Rtheta / cos(lat);
    }

    if (fabs(lon) > M_PI) return(false);

    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= TWO_PI;
    else if (lon < -M_PI) lon += TWO_PI;

    return(true);
}

bool
ProjectionBonne::sphericalToPixel(double lon, double lat,
                                  double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);

    double X, Y;
    if (sfl_)
    {
        X = lon * cos(lat);
        Y = lat;
    }
    else
    {
        double Rtheta = Y0_ - lat;
        double Aphi = lon * cos(lat) / Rtheta;
        
        X = Rtheta * sin(Aphi);
        Y = Y0_ - Rtheta * cos(Aphi);
    }

    x = width_ * (X * scale_/TWO_PI + 0.5);
    y = height_ * (0.5 - Y * scale_/M_PI);

    x += (centerX_ - width_/2);
    y += (yOffset_ + centerY_ - height_/2);

    if (y < 0 || y >= height_) return(false);

    return(true);
}
