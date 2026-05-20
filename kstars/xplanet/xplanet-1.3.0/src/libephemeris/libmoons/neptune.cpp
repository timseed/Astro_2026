#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace std;

/*
  Ephemerides for Triton and Nereid are described in Jacobson, 
  Astron. Astrophys. 231, 241-250 (1990)
*/

#include "body.h"
#include "xpUtil.h"

void
nepsat(const double jd, const body b, double &X, double &Y, double &Z)
{
    double td;       // Julian days from reference date
    double ty;       // Julian years from reference date
    double tc;       // Julian centuries from reference date

    double a;        // semimajor axis
    double L;        // mean longitude
    double e;        // eccentricity
    double w;        // longitude of periapse
    double i;        // inclination of orbit
    double o;        // longitude of ascending node

    double ma;       // mean anomaly

    double N;        // node of the orbital reference plane on the
                     // Earth equator B1950
    double J;        // inclination of orbital reference plane with
                     // respect to the Earth equator B1950

    switch (b)
    {
    case TRITON:
        td = jd - 2433282.5;
        ty = td/365.25;
        tc = ty/100;

        a = 354611.773;
        L = (49.85334766 + 61.25726751 * td) * deg_to_rad;
        e = 0.0004102259410;
        i = 157.6852321 * deg_to_rad;
        o = (151.7973992 + 0.5430763965 * ty) * deg_to_rad;

        w = (236.7318362 + 0.5295275852 * ty) * deg_to_rad;

        ma = L - w;

        w += o;

        // inclination and node of the invariable plane on the Earth
        // equator of 1950
        J = (90 - 42.51071244) * deg_to_rad;
        N = (90 + 298.3065940) * deg_to_rad;
    
        break;
    case NEREID:
        td = jd - 2433680.5;
        tc = td/36525;

        a = 5511233.255;
        L = (251.14984688 + 0.9996465329 * td) * deg_to_rad;
        e = 0.750876291;
        i = 6.748231850 * deg_to_rad;
        o = (315.9958928 - 3.650272562 * tc) * deg_to_rad;

        w = (251.7242240 + 0.8696048083 * tc) * deg_to_rad;

        ma = L - w;

        w -= o;

        // inclination and node of Neptune's orbit on the Earth
        // equator of 1950
        J = 22.313 * deg_to_rad;
        N = 3.522 * deg_to_rad;
        break;
    default:
        xpExit("Unknown Neptune satellite\n", __FILE__, __LINE__);
    }

    double E = kepler(e, ma);

    // convert semi major axis from km to AU
    a /= AU_to_km;

    // rectangular coordinates on the orbit plane, x-axis is toward
    // pericenter
    X = a * (cos(E) - e);
    Y = a * sqrt(1 - e*e) * sin(E);
    Z = 0;

    // rotate towards ascending node of the orbit
    rotateZ(X, Y, Z, -w);

    // rotate towards orbital reference plane
    rotateX(X, Y, Z, -i);

    // rotate towards ascending node of the orbital reference plane on
    // the Earth equator B1950
    rotateZ(X, Y, Z, -o);

    // rotate towards Earth equator B1950
    rotateX(X, Y, Z, -J);

    // rotate to vernal equinox
    rotateZ(X, Y, Z, -N);

    // precess to J2000
    precessB1950J2000(X, Y, Z);
}
