#include <cmath>
#include <cstdio>
#include <iostream>

#include "Separation.h"
#include "View.h"
#include "xpUtil.h"

using namespace std;

Separation::Separation(const double oX, const double oY, const double oZ,
                       const double tX, const double tY, const double tZ, 
                       const double sX, const double sY, const double sZ) 
{
    view_ = new View(tX, tY, tZ, oX, oY, oZ, sX, sY, sZ, 0, 0);
    view_->RotateToViewCoordinates(oX, oY, oZ, oX_, oY_, oZ_);
    view_->RotateToViewCoordinates(tX, tY, tZ, tX_, tY_, tZ_);
    view_->RotateToViewCoordinates(sX, sY, sZ, sX_, sY_, sZ_);

    oR_ = sqrt(oY_ * oY_ + oZ_ * oZ_);
    sR_ = sqrt(sY_ * sY_ + sZ_ * sZ_);
}

Separation::~Separation()
{
    delete view_;
}

void
Separation::getOrigin(double &oX, double &oY, double &oZ)
{
    view_->RotateToXYZ(oX_, oY_, oZ_, oX, oY, oZ);
}

/*
 Given an angle sep, calculate the observer position using the
 bisection method.

 If the observer is closer to the target than target 2, then any
 separation angle is possible (e.g. observer = Venus, target = Sun,
 target 2 = Earth).  The separation function increases monotonically
 from -PI to PI as alpha0 increases.  At alpha0 + PI the separation
 function is 0.

 If the observer is farther from the target than target 1, there is a
 maximum separation possible (e.g. observer = Earth, target = Sun,
 target 2 = Venus).  In this case the separation function looks like
 this:

 angle - alpha0     sep
 0                  0
 acos(sR/oR)       -max
 PI                 0
 2 PI-acos(sR/oR)   max
 2 PI               0

 In this case, look for the angle that gives the required separation
 close to 0 or 2 PI, depending on the sign of the separation.
*/
void
Separation::setSeparation(double sep)
{
    const double alpha0 = atan2(sY_, sZ_);

    double x0 = alpha0;
    double x1 = alpha0 + TWO_PI;

    if (sep < 0)
        x1 = alpha0 + M_PI;
    else
        x0 = alpha0 + M_PI;

    if (oR_ > sR_)
    {
        double max_sep = asin(sR_/oR_);
        if (sep > max_sep)
        {
            sep = max_sep;
            printf("sep = %14.6f, maximum separation is %14.6f\n", 
                   sep/deg_to_rad, max_sep / deg_to_rad);
            calcSeparation(alpha0 + M_PI_2);
            return;
        }

        if (sep < 0)
            x1 = alpha0 + acos(sR_/oR_);
        else
            x0 = alpha0 + TWO_PI - acos(sR_/oR_);
    }

    double f0 = calcSeparation(x0) - sep;

    if (x0 == alpha0 && f0 > 0)
        f0 = -(calcSeparation(x0) + sep);

    if (f0 > 0)
    {
        double tmp = x0;
        x0 = x1;
        x1 = tmp;
    }

    for (int i = 0; i < 200; i++)
    {
        double xmid = 0.5 * (x0 + x1);
        double fmid = calcSeparation(xmid) - sep;

        if (fmid > 0)
            x1 = xmid;
        else
            x0 = xmid;

        if (fabs(x0-x1) < 1e-10 || fmid == 0.0) break;
    }
}

// Assume target 1 is at (0, 0, 0) and target 2 is at (0, y2, z2).
// Supply an angle, and calculate the new observer location (0, y, z)
// and the separation between the two targets.
double
Separation::calcSeparation(const double alpha)
{
    oX_ = 0;
    oY_ = oR_ * sin(alpha);
    oZ_ = oR_ * cos(alpha);

    const double t[3] = { tX_ - oX_, tY_ - oY_, tZ_ - oZ_ };
    const double s[3] = { sX_ - oX_, sY_ - oY_, sZ_ - oZ_ };

    double separation = 0;
    const double cos_sep = ndot(t, s);
    if (cos_sep >= 1.0)
        separation = 0;
    else if (cos_sep <= -1.0)
        separation = M_PI;
    else
        separation = acos(cos_sep);

    double c[3];
    cross(s, t, c);

    if (c[0] > 0) separation *= -1;

    return(separation);
}
