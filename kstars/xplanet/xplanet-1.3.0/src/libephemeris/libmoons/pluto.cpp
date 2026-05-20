#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace std;

/*
  Ephemeris for Charon is from Tholen, D.J. (1985) Astron. J., 90,
  2353-2359
*/

#include "body.h"
#include "xpUtil.h"

void
plusat(const double jd, double &X, double &Y, double &Z)
{
    double td = jd - 2445000.5;                      // Julian days from
                                                     // reference date
    const double a = 19130 / AU_to_km;               // semimajor axis (km)
    
    const double n = 360 / 6.38723;                  // mean motion (degrees/day)
    const double E = (78.6 + n * td) * deg_to_rad;   // eccentric anomaly
    const double i = 94.3 * deg_to_rad;              // inclination of orbit
    const double o = 223.7 * deg_to_rad;             // longitude of ascending node

    // rectangular coordinates on the orbit plane, x-axis is toward
    // pericenter
    X = a * cos(E);
    Y = a * sin(E);
    Z = 0;

    // rotate towards Earth equator B1950
    rotateX(X, Y, Z, -i);

    // rotate to vernal equinox
    rotateZ(X, Y, Z, -o);

    // precess to J2000
    precessB1950J2000(X, Y, Z);
}
