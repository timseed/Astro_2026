#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace std;

/*
  Ephemerides for Phobos and Deimos are described in Sinclair,
  Astron. Astrophys. 220, 321-328 (1989)
*/

#include "body.h"
#include "xpUtil.h"

void
marsat(const double jd, const body b, double &X, double &Y, double &Z)
{
    const double td = jd - 2441266.5;
    const double ty = td/365.25;

    double a;        // semimajor axis
    double e;        // eccentricity
    double I;        // inclination of orbit to Laplacian plane

    double L;        // mean longitude
    double P;        // longitude of pericenter
    double K;        // longitude of node of orbit on Laplacian plane
    
    double N;        // node of the Laplacian plane on the Earth
                     // equator B1950
    double J;        // inclination of Laplacian plane with respect to
                     // the Earth equator B1950

    switch (b)
    {
    case PHOBOS:
        a = 9379.40;
        e = 0.014979;
        I = 1.1029 * deg_to_rad;
        L = (232.412 + 1128.8445566 * td + 0.001237 * ty * ty) * deg_to_rad;
        P = (278.96 + 0.435258 * td) * deg_to_rad;
        K = (327.90 - 0.435330 * td) * deg_to_rad;
        N = (47.386 - 0.00140 * ty) * deg_to_rad;
        J = (37.271 + 0.00080 * ty) * deg_to_rad;
        break;
    case DEIMOS:
    {
        a = 23461.13;
        e = 0.000391;
        I = 1.7901 * deg_to_rad;
        P = (111.7 + 0.017985 * td) * deg_to_rad;
        L = (28.963 + 285.1618875 * td) * deg_to_rad;
        K = (240.38 - 0.018008 * td) * deg_to_rad;
        N = (46.367 - 0.00138 * ty) * deg_to_rad;
        J = (36.623 + 0.00079 * ty) * deg_to_rad;
        
        double dL = -0.274 * sin(K - 43.83 * deg_to_rad) * deg_to_rad;
        L += dL;
    }
    break;
    default:
        xpExit("Unknown Mars satellite\n", __FILE__, __LINE__);
    }
    double ma = L - P;
    double E = kepler(e, ma);

    // convert semi major axis from km to AU
    a /= AU_to_km;

    // rectangular coordinates on the orbit plane, x-axis is toward
    // pericenter
    X = a * (cos(E) - e);
    Y = a * sqrt(1 - e*e) * sin(E);
    Z = 0;

    // longitude of pericenter measured from ascending node of the
    // orbit on the Laplacian plane
    const double omega = P - (K + N);

    // rotate towards ascending node of the orbit on the Laplacian
    // plane
    rotateZ(X, Y, Z, -omega);

    // rotate towards Laplacian plane
    rotateX(X, Y, Z, -I);

    // rotate towards ascending node of the Laplacian plane on the
    // Earth equator B1950
    rotateZ(X, Y, Z, -K);

    // rotate towards Earth equator B1950
    rotateX(X, Y, Z, -J);

    // rotate to vernal equinox
    rotateZ(X, Y, Z, -N);

    // precess to J2000
    precessB1950J2000(X, Y, Z);
}
