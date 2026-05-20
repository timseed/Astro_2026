#include <cmath>
using namespace std;

#include "Options.h"
#include "xpUtil.h"

#include "ProjectionOrthographic.h"

ProjectionOrthographic::ProjectionOrthographic(const int f, const int w, const int h)
    : ProjectionBase(f, w, h)
{
    isWrapAround_ = false;

    Options *options = Options::getInstance();

    dispScale_ = radius_ * height_;
    setRange(options->Range());

    buildPhotoTable();
}

ProjectionOrthographic::~ProjectionOrthographic() 
{
    destroyPhotoTable();
}

void
ProjectionOrthographic::setRange(const double range)
{
    P = range;
    Psq = P*P;
    Pp1 = (P + 1);
    Pm1 = (P - 1);
    PPm1 = P * Pm1;
    Pm1sq = Pm1 * Pm1;

    dispScale_ *= sqrt(Pp1/Pm1);
}

bool
ProjectionOrthographic::pixelToSpherical(const double x, const double y, 
                                         double &lon, double &lat)
{
    const double X = (x - centerX_)/dispScale_;
    const double Y = (centerY_ - y)/dispScale_;

    const double rho2 = X*X + Y*Y;
    if (rho2 > 1) return(false);

    const double rho = sqrt(rho2);

    if (rho == 0)
    {
        lat = 0; 
        lon = 0;
    }
    else
    {
        double arg = Pm1*(Pm1 - rho2 * Pp1);
        if (arg < 0) return(false);

        const double N = rho * (PPm1 - sqrt(arg));
        const double D = (Pm1sq + rho2);

        const double sinc = N/D;
        const double cosc = sqrt(1 - sinc*sinc);

        arg = Y * sinc / rho;
        if (fabs(arg) > 1) return(false);
        
        lat = asin(arg);
        lon = atan2(X * sinc, rho * cosc);
    }

    // This is the cosine of the observer-planet center-normal angle
    const double cosa = cos(lat) * cos(lon);
    const double sina_sq = 1 - cosa*cosa;
    
    // This is the distance from the observer to the point on the surface
    const double dist_sq = Psq - 2 * P * cosa + 1;
    
    // This is the angle we want: observer-surface-normal
    const double sinb_sq = Psq / dist_sq * sina_sq;
    const double cosb = sqrt(1 - sinb_sq);
    
    darkening_ = getPhotoFunction(fabs(cosb)); 

    if (rotate_) RotateXYZ(lat, lon);

    if (lon > M_PI) lon -= 2*M_PI;
    else if (lon < -M_PI) lon += 2*M_PI;

    return(true);
}

bool
ProjectionOrthographic::sphericalToPixel(double lon, double lat,
                                         double &x, double &y) const
{
    if (rotate_) RotateZYX(lat, lon);

    const double cosc = cos(lat) * cos(lon);
    if (cosc < 0) return(false);

    const double k = (P - 1) / (P - cosc);

    const double X = k * cos(lat) * sin(lon);
    const double Y = k * sin(lat);

    x = X * dispScale_ + centerX_;
    if (x < 0 || x >= width_) return(false);

    y = centerY_ - Y * dispScale_;
    if (y < 0 || y >= height_) return(false);

    if (P*cosc < 1) 
    {
        double dist = sqrt(x*x + y*y);
        if (dist < dispScale_) return(false);
    }

    return(true);
}
