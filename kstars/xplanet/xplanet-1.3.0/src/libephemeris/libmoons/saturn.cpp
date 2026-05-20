#include <cmath>
#include <cstdlib>
using namespace std;

#include "body.h"
#include "xpUtil.h"

#include "tass17.h"

/*
  The TASS theory of motion by Vienne and Duriez is described in
  (1995, A&A 297, 588-605) for the inner six satellites and Iapetus
  and in (1997, A&A 324, 366-380) for Hyperion.  Much of this code is
  translated from the TASS17 FORTRAN code which is at
  ftp://ftp.bdl.fr/pub/ephem/satel/tass17

  Orbital elements for Phoebe are from the Explanatory Supplement and
  originally come from Zadunaisky (1954).
 */

static void
calcLon(const double jd, double lon[])
{
    const double t = (jd - 2444240)/365.25;

    for (int is = 0; is < 7; is++)
    {
        lon[is] = 0;
        for (int i = 0; i < ntr[is][4]; i++)
            lon[is] += series[is][1][i][0] * sin(series[is][1][i][1] 
                                                 + t * series[is][1][i][2]);
    }
}

static void
calcElem(const double jd, const int is, const double lon[], double elem[6])
{
    const double t = (jd - 2444240)/365.25;

    double s = 0;

    for (int i = 0; i < ntr[is][0]; i++)
    {
        double phase = series[is][0][i][1];
        for (int j = 0; j < 7; j++)
            phase += iks[is][0][i][j] * lon[j];
        s += series[is][0][i][0] * cos(phase + t*series[is][0][i][2]);
    }

    elem[0] = s;

    s = lon[is] + al0[is];
    for (int i = ntr[is][4]; i < ntr[is][1]; i++)
    {
        double phase = series[is][1][i][1];
        for (int j = 0; j < 7; j++)
            phase += iks[is][1][i][j] * lon[j];
        s += series[is][1][i][0] * sin(phase + t*series[is][1][i][2]);
    }
    s += an0[is]*t;
    elem[1] = atan2(sin(s), cos(s));

    double s1 = 0;
    double s2 = 0;
    for (int i = 0; i < ntr[is][2]; i++)
    {
        double phase = series[is][2][i][1];
        for (int j = 0; j < 7; j++)
            phase += iks[is][2][i][j] * lon[j];
        s1 += series[is][2][i][0] * cos(phase + t*series[is][2][i][2]);
        s2 += series[is][2][i][0] * sin(phase + t*series[is][2][i][2]);
    }
    elem[2] = s1;
    elem[3] = s2;

    s1 = 0;
    s2 = 0;
    for (int i = 0; i < ntr[is][3]; i++)
    {
        double phase = series[is][3][i][1];
        for (int j = 0; j < 7; j++)
            phase += iks[is][3][i][j] * lon[j];
        s1 += series[is][3][i][0] * cos(phase + t*series[is][3][i][2]);
        s2 += series[is][3][i][0] * sin(phase + t*series[is][3][i][2]);
    }
    elem[4] = s1;
    elem[5] = s2;
}

static void
elemHyperion(const double jd, double elem[6])
{
    const double T0 = 2451545.0;
    const double AMM7 = 0.2953088138695055;

    const double T = jd - T0;

    elem[0] = -0.1574686065780747e-02;
    for (int i = 0; i < NBTP; i++)
    {
        const double wt = T*P[i][2] + P[i][1];
        elem[0] += P[i][0] * cos(wt);
    }

    elem[1] = 0.4348683610500939e+01;
    for (int i = 0; i < NBTQ; i++)
    {
        const double wt = T*Q[i][2] + Q[i][1];
        elem[1] += Q[i][0] * sin(wt);
    }

    elem[1] += AMM7*T;
    elem[1] = fmod(elem[1], 2*M_PI);
    if (elem[1] < 0) elem[1] += 2*M_PI;

    for (int i = 0; i < NBTZ; i++)
    {
        const double wt = T*Z[i][2] + Z[i][1];
        elem[2] += Z[i][0] * cos(wt);
        elem[3] += Z[i][0] * sin(wt);
    }
        
    for (int i = 0; i < NBTZT; i++)
    {
        const double wt = T*ZT[i][2] + ZT[i][1];
        elem[4] += ZT[i][0] * cos(wt);
        elem[5] += ZT[i][0] * sin(wt);
    }
        
}

void
satsat(const double jd, const body b, 
       double &X, double &Y, double &Z)
{
    double elem[6] = { 0, 0, 0, 0, 0, 0 };

    double aam;   // mean motion, in radians per day
    double tmas;        // mass, in Saturn masses
        
    if (b == PHOEBE)
    {
        const double t = jd - 2433282.5;
        const double T = t/365.25;

        const double axis = 0.0865752;
        const double lambda = (277.872 - 0.6541068 * t) * deg_to_rad;
        const double e = 0.16326;
        const double lp = (280.165 - 0.19586 * T) * deg_to_rad;
        const double i = (173.949 - 0.020 * T) * deg_to_rad - M_PI;  // retrograde orbit
        const double omega = (245.998 - 0.41353 * T) * deg_to_rad;

        const double M = lambda - lp;
        const double E = kepler(e, M);

        // rectangular coordinates on the orbit plane, x-axis is toward
        // pericenter
        X = axis * (cos(E) - e);
        Y = axis * sqrt(1 - e*e) * sin(E);
        Z = 0;

        // rotate towards ascending node of the orbit on the ecliptic
        // and equinox of 1950
        rotateZ(X, Y, Z, -(lp - omega));

        // rotate towards ecliptic
        rotateX(X, Y, Z, -i);

        // rotate to vernal equinox
        rotateZ(X, Y, Z, -omega);

        // rotate to earth equator B1950
        const double eps = 23.4457889 * deg_to_rad;
        rotateX(X, Y, Z, -eps);
        
        // precess to J2000
        precessB1950J2000(X, Y, Z);
    }
    else if (b == HYPERION)
    {
        elemHyperion(jd, elem);

        aam = 0.2953088138695000E+00 * 365.25;
        tmas = 1/0.3333333333333000E+08;
    }
    else
    {
        int index = 0;

        switch (b)
        {
        case MIMAS:
            index = 0;
            break;
        case ENCELADUS:
            index = 1;
            break;
        case TETHYS:
            index = 2;
            break;
        case DIONE:
            index = 3;
            break;
        case RHEA:
            index = 4;
            break;
        case TITAN:
            index = 5;
            break;
        case IAPETUS:
            index = 6;
            break;
        default:
            xpExit("Unknown Saturn satellite\n", __FILE__, __LINE__);
        }

        double lon[7];
        calcLon(jd, lon);
        calcElem(jd, index, lon, elem);

        aam = am[index] * 365.25;
        tmas = 1/tam[index];
    }    

    if (b != PHOEBE)
    {
        const double GK = 0.01720209895;
        const double TAS = 3498.790;
        const double GK1 = (GK * 365.25) * (GK * 365.25) / TAS;
    
        const double amo = aam * (1 + elem[0]);
        const double rmu = GK1 * (1 + tmas);
        const double dga = pow(rmu/(amo*amo), 1./3.);
        const double rl = elem[1];
        const double rk = elem[2];
        const double rh = elem[3];

        double corf = 1;
        double fle = rl - rk * sin(rl) + rh * cos(rl);
        while (fabs(corf) > 1e-14)
        {
            const double cf = cos(fle);
            const double sf = sin(fle);
            corf = (rl - fle + rk*sf - rh*cf)/(1 - rk*cf - rh*sf);
            fle += corf;
        }

        const double cf = cos(fle);
        const double sf = sin(fle);

        const double dlf = -rk * sf + rh * cf;
        const double rsam1 = -rk * cf - rh * sf;
        const double asr = 1/(1 + rsam1);
        const double phi = sqrt(1 - rk*rk - rh*rh);
        const double psi = 1/(1+phi);

        const double x1 = dga * (cf - rk - psi * rh * dlf);
        const double y1 = dga * (sf - rh + psi * rk * dlf);
        const double vx1 = amo * asr * dga * (-sf - psi * rh * rsam1);
        const double vy1 = amo * asr * dga * ( cf + psi * rk * rsam1);
    
        const double dwho = 2 * sqrt(1 - elem[5] * elem[5] - elem[4] * elem[4]);
        const double rtp = 1 - 2 * elem[5] * elem[5];
        const double rtq = 1 - 2 * elem[4] * elem[4];
        const double rdg = 2 * elem[5] * elem[4];

        const double X1 = x1 * rtp + y1 * rdg;
        const double Y1 = x1 * rdg + y1 * rtq;
        const double Z1 = (-x1 * elem[5] + y1 * elem[4]) * dwho;

        const double AIA = 28.0512 * deg_to_rad;
        const double OMA = 169.5291 * deg_to_rad;

        const double ci = cos(AIA);
        const double si = sin(AIA);
        const double co = cos(OMA);
        const double so = sin(OMA);

        X = co * X1 - so * ci * Y1 + so * si * Z1;
        Y = so * X1 + co * ci * Y1 - co * si * Z1;
        Z = si * Y1 + ci * Z1;

        // rotate to earth equator J2000
        const double eps = 23.4392911 * deg_to_rad;
        rotateX(X, Y, Z, -eps);
/*      
        const double VX1 = vx1 * rtp + vy1 * rdg;
        const double VY1 = vx1 * rdg + vy1 * rtq;
        const double VZ1 = (-vx1 * elem[5] + vy1 * elem[4]) * dwho;

        Vx = co * VX1 - so * ci * VY1 + so * si * VZ1;
        Vy = so * VX1 + co * ci * VY1 - co * si * VZ1;
        Vz = si * VY1 + ci * VZ1;
*/
    }
}

#if 0
int 
main(int argc, char **argv)
{
    double X, Y, Z;

    body b;

    b = MIMAS ;
    satsat(2421677.4, b, X, Y, Z);
    satsat(2441692.3, b, X, Y, Z);
    satsat(2445106.3, b, X, Y, Z);

    b = ENCELADUS  ;
    satsat(2406147.5, b, X, Y, Z);
    satsat(2441699.9, b, X, Y, Z);
    satsat(2444714., b, X, Y, Z);

    b = TETHYS ;
    satsat(2409977.4, b, X, Y, Z);
    satsat(2432270.9, b, X, Y, Z);
    satsat(2445814., b, X, Y, Z);

    b = DIONE  ;
    satsat(2406477.5, b, X, Y, Z);
    satsat(2441257.8, b, X, Y, Z);
    satsat(2445820.6, b, X, Y, Z);

    b = RHEA ;
    satsat(2405824.5, b, X, Y, Z);
    satsat(2432236.8, b, X, Y, Z);
    satsat(2445814., b, X, Y, Z);

    b = TITAN  ;
    satsat(2440512.6, b, X, Y, Z);
    satsat(2443569.3, b, X, Y, Z);
    satsat(2445061.3, b, X, Y, Z);

    b = HYPERION;
    satsat(2406327.6, b, X, Y, Z);
    satsat(2443128.7, b, X, Y, Z);
    satsat(2445720.1, b, X, Y, Z);

    b = IAPETUS;
    satsat(2406216.6, b, X, Y, Z);
    satsat(2443179.7, b, X, Y, Z);
    satsat(2445815.1, b, X, Y, Z);
}

#endif
