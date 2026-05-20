/*
  This code is adapted from the Fortran code "planetap.for" at
  http://adc.gsfc.nasa.gov/adc-cgi/cat.pl?/catalogs/6/6066/

  The algorithms are from
  Numerical expressions for precession formulae and mean elements
  for the Moon and the planets.
  Simon J.L., Bretagnon P., Chapront J., Chapront-Touze M.,
  Francou G., Laskar J.  <Astron. Astrophys. 282, 663 (1994)>

  Precision :
  Over the interval 1800-2200, the precision is :
  for the longitude of Mercury          4"
  for the longitude of Venus            5"
  for the longitude of E-M barycenter   6"
  for the longitude of Mars            17"
  for the longitude of Jupiter         71"
  for the longitude of Saturn          81"
  for the longitude of Uranus          86"
  for the longitude of Neptune         11"
  Over the interval 1000-3000, the precision is better than 1.5
  times the precision over 1800-2200.

*/

#include "xpUtil.h"

#include "EphemerisLow.h"
using namespace std;

extern 
void pluto(const double JD, double &Px, double &Py, double &Pz, 
           double &Vx, double &Vy, double &Vz);

EphemerisLow::EphemerisLow() : Ephemeris()
{
}

EphemerisLow::~EphemerisLow()
{
}

void
EphemerisLow::GetHeliocentricXYZ(const body b, const double tjd, 
                                 double &Px, double &Py, double &Pz)
{
    double a[8][3] = { { 0.3870983098,             0.,        0. },
                       { 0.7233298200,             0.,        0. },
                       { 1.0000010178,             0.,        0. },
                       { 1.5236793419,         3.e-10,        0. },
                       { 5.2026032092,     19132.e-10,  -39.e-10 },
                       { 9.5549091915,  -0.0000213896,  444.e-10 },
                       { 19.2184460618,    -3716.e-10,  979.e-10 },
                       { 30.1103868694,   -16635.e-10,  686.e-10} };

    double dlm[8][3] = { { 252.25090552, 5381016286.88982,  -1.92789 },
                         { 181.97980085, 2106641364.33548,   0.59381 },
                         { 100.46645683, 1295977422.83429,  -2.04411 },
                         { 355.43299958,  689050774.93988,   0.94264 },
                         { 34.35151874,   109256603.77991, -30.60378 },
                         { 50.07744430,    43996098.55732,  75.61614 },
                         { 314.05500511,   15424811.93933,  -1.75083 },
                         { 304.34866548,    7865503.20744,   0.21103 } };

    double e[8][3] = { { 0.2056317526,  0.0002040653,   -28349.e-10 },
                       { 0.0067719164, -0.0004776521,    98127.e-10 },
                       { 0.0167086342, -0.0004203654, -0.0000126734 },
                       { 0.0934006477,  0.0009048438,   -80641.e-10 },
                       { 0.0484979255,  0.0016322542, -0.0000471366 },
                       { 0.0555481426, -0.0034664062, -0.0000643639 },
                       { 0.0463812221, -0.0002729293,  0.0000078913 },
                       { 0.0094557470,  0.0000603263,            0. } };

    double pi[8][3] = {  { 77.45611904,   5719.11590,   -4.83016 },
                         { 131.56370300,   175.48640, -498.48184 },
                         { 102.93734808, 11612.35290,   53.27577 },
                         { 336.06023395, 15980.45908,  -62.32800 },
                         { 14.33120687,   7758.75163,  259.95938 },
                         { 93.05723748,  20395.49439,  190.25952 },
                         { 173.00529106,  3215.56238,  -34.09288 },
                         { 48.12027554,   1050.71912,   27.39717 } };

    double dinc[8][3] = { { 7.00498625, -214.25629,    0.28977 },
                          { 3.39466189,   -30.84437,  -11.67836 },
                          { 0.,          469.97289,   -3.35053 },
                          { 1.84972648, -293.31722,   -8.11830 },
                          { 1.30326698,  -71.55890,   11.95297 },
                          { 2.48887878,   91.85195,  -17.66225 },
                          { 0.77319689,  -60.72723,    1.25759 },
                          { 1.76995259,    8.12333,    0.08135 } };

    double omega[8][3] = {  { 48.33089304,  -4515.21727,  -31.79892 },
                            { 76.67992019, -10008.48154,  -51.32614 },
                            { 174.87317577,  -8679.27034,   15.34191 },
                            { 49.55809321, -10620.90088, -230.57416 },
                            { 100.46440702,   6362.03561,  326.52178 },
                            { 113.66550252,  -9240.19942,  -66.23743 },
                            { 74.00595701,   2669.15033,  145.93964 },
                            { 131.78405702,   -221.94322 ,  -0.78728} };

    int kp[8][9] = { { 69613, 75645, 88306, 59899, 15746, 71087, 142173,  3086,    0 },
                     { 21863, 32794, 26934, 10931, 26250, 43725,  53867, 28939,    0 },
                     { 16002, 21863, 32004, 10931, 14529, 16368,  15318, 32794,    0 },
                     { 6345,   7818, 15636,  7077,  8184, 14163,   1107,  4872,    0 },
                     { 1760,   1454,  1167,   880,   287,  2640,     19,  2047, 1454 },
                     { 574,      0,   880,   287,    19,  1760,   1167,   306,  574 },
                     { 204,      0,   177,  1265,     4,   385,    200,   208,  204 },
                     { 0,    102,   106,     4,    98,  1367,    487,   204,    0} };

    double ca[8][9] = { { 4,    -13,    11,    -9,    -9,    -3,    -1,     4,    0 },
                        { -156,     59,   -42,     6,    19,   -20,   -10,   -12,    0 },
                        { 64,   -152,    62,    -8,    32,   -41,    19,   -11,    0 },
                        { 124,    621,  -145,   208,    54,   -57,    30,    15,    0 },
                        { -23437,  -2634,  6601,  6259, -1507, -1821,  2620, -2115,-1489 },
                        { 62911,-119919, 79336, 17814,-24241, 12068,  8306, -4893, 8902 },
                        { 389061,-262125,-44088,  8387,-22976, -2093,  -615, -9720, 6633 },
                        { -412235,-157046,-31430, 37817, -9740,   -13, -7449,  9644,    0} };

    double sa[8][9] = { { -29,     -1,     9,     6,    -6,     5,     4,     0,    0 },
                        { -48,   -125,   -26,   -37,    18,   -13,   -20,    -2,    0 },
                        { -150,    -46,    68,    54,    14,    24,   -28,    22,    0 },
                        { -621,    532,  -694,   -20,   192,   -94,    71,   -73,    0 },
                        { -14614, -19828, -5869,  1881, -4372, -2255,   782,   930,  913 },
                        { 139737,      0, 24667, 51123, -5102,  7429, -4095, -1976,-9566 },
                        { -138081,      0, 37205,-49039,-41901,-33872,-27037,-12474,18797 },
                        { 0,  28492,133236, 69654, 52322,-49577,-26430, -3593,    0} };

    int kq[8][10] = { { 3086,  15746, 69613, 59899, 75645, 88306,  12661,  2658,  0,   0 },
                      { 21863,  32794, 10931,    73,  4387, 26934,   1473,  2157,  0,   0 },
                      { 10,  16002, 21863, 10931,  1473, 32004,   4387,    73,  0,   0 },
                      { 10,   6345,  7818,  1107, 15636,  7077,   8184,   532, 10,   0 },
                      { 19,   1760,  1454,   287,  1167,   880,    574,  2640, 19,1454 },
                      { 19,    574,   287,   306,  1760,    12,     31,    38, 19, 574 },
                      { 4,    204,   177,     8,    31,   200,   1265,   102,  4, 204 },
                      { 4,    102,   106,     8,    98,  1367,    487,   204,  4, 102} };

    double cl[8][10] = { { 21,   -95, -157,   41,   -5,   42,   23,   30,     0,    0 },
                         { -160,  -313, -235,   60,  -74,  -76,  -27,   34,     0,    0 },
                         { -325,  -322,  -79,  232,  -52,   97,   55,  -41,     0,    0 },
                         { 2268,  -979,  802,  602, -668,  -33,  345,  201,   -55,    0 },
                         { 7610, -4997,-7689,-5841,-2617, 1115, -748, -607,  6074,  354 },
                         { -18549, 30125,20012, -730,  824,   23, 1289, -352,-14767,-2062 },
                         { -135245,-14594, 4197,-4030,-5630,-2898, 2540, -306,  2939, 1986 },
                         { 89948,  2103, 8963, 2695, 3682, 1648,  866, -154, -1963, -283} };

    double sl[8][10] = { { -342,   136,  -23,   62,   66,  -52,  -33,   17,     0,    0 },
                         { 524,  -149,  -35,  117,  151,  122,  -71,  -62,     0,    0 },
                         { -105,  -137,  258,   35, -116,  -88, -112,  -80,     0,    0 },
                         { 854,  -205, -936, -240,  140, -341,  -97, -232,   536,    0 },
                         { -56980,  8016, 1012, 1448,-3024,-3710,  318,  503,  3767,  577 },
                         { 138606,-13478,-4964, 1441,-1319,-1482,  427, 1236, -9167,-1918 },
                         { 71234,-41116, 5334,-4935,-1848,   66,  434,-1748,  3780, -701 },
                         { -47645, 11647, 2166, 3194,  679,    0, -244, -419, -2531,   48} };

    double rmas[8] = { 6023600, 408523.5, 328900.5, 3098710, 1047.355, 3498.5, 22869, 19314 };


    double Vx = 0, Vy = 0, Vz = 0;

    int index = -1;
    switch (b)
    {
    case SUN:
        Px = 0;
        Py = 0;
        Pz = 0;
        Vx = 0;
        Vy = 0;
        Vz = 0;
        return;
        break;
    case MERCURY:
        index = 0; 
        break;
    case VENUS:
        index = 1; 
        break;
    case EARTH:
        index = 2;
        break;
    case MARS:
        index = 3;
        break;
    case JUPITER:
        index = 4;
        break;
    case SATURN:
        index = 5;
        break;
    case URANUS:
        index = 6;
        break;
    case NEPTUNE:
        index = 7;
        break;
    case PLUTO:
        pluto(tjd, Px, Py, Pz, Vx, Vy, Vz);
        break;
    default:
        break;
    }

    if (b != PLUTO)
    {
        calcHeliocentricXYZ(tjd, 1/rmas[index], a[index], dlm[index], 
                            e[index], pi[index], dinc[index], omega[index], 
                            kp[index], ca[index], sa[index], 
                            kq[index], cl[index], sl[index],
                            Px, Py, Pz, Vx, Vy, Vz);

        // rotate to earth equator J2000
        const double eps = 23.4392911 * deg_to_rad;

        const double sEps = sin(eps);
        const double cEps = cos(eps);

        const double Y0 = Py;
        const double Z0 = Pz;
        
        Py = Y0 * cEps - Z0 * sEps;
        Pz = Z0 * cEps + Y0 * sEps;
    }
}

void
EphemerisLow::kepler(const double al, const complex<double> &z, 
                     const double u, complex<double> &zto, double &r)
{
    const double ex = abs(z);
    const double ex2 = ex*ex;
    const double ex3 = ex2*ex;

    double e = (al + (ex - ex3/8)*sin(al) + 0.5 * ex2 * sin(2*al) 
                + 0.375 * ex3 * sin(3*al));

    complex<double> z1 = conj(z);

    for (int k = 0; k < 10; k++)
    {
        const complex<double> z2(0, e);
        const complex<double> zteta = exp(z2);

        const complex<double> z3 = z1*zteta;

        const double dl = al - e + z3.imag();
        r = 1 - z3.real();
        if (fabs(dl) < 1e-12)
        {
            z1 = u * z * z3.imag();
            const complex<double> z2(z1.imag(), -z1.real());
            zto = (-z + zteta + z2)/r;
            return;
        }
        e += dl/r;
    }
    xpWarn("Kepler: can't converge\n", __FILE__, __LINE__);
}

void
EphemerisLow::ellipx(const double a, const double dlm, const double e, 
                     const double p, const double dia, const double omega, 
                     const double dmas, 
                     double &Px, double &Py, double &Pz,
                     double &Vx, double &Vy, double &Vz)
{
    const double xa = a;
    const double xl = dlm;
    const double xk = e*cos(p);
    const double xh = e*sin(p);
    const double xq = sin(dia/2) * cos(omega);
    const double xp = sin(dia/2) * sin(omega);

    const double asr = 648000/M_PI;
    const double gk = 3548.1876069651;
    const double xfi = sqrt(1 - xk*xk - xh*xh);
    const double xki = sqrt(1 - xq*xq - xp*xp);

    const double u = 1/(1+xfi);
    
    complex<double> z(xk, xh);
    complex<double> zto;
    double r;

    kepler(xl, z, u, zto, r);

    const double xcw = zto.real();
    const double xsw = zto.imag();
    
    const double xm = xp*xcw - xq*xsw;
    const double xxx = xa*r;

    Px = xxx*(xcw - 2*xp*xm);
    Py = xxx*(xsw + 2*xq*xm);
    Pz = -2*xxx*xki*xm;

    const double xms = xa*(xh+xsw)/xfi;
    const double xmc = xa*(xk+xcw)/xfi;
    const double xn = gk*sqrt((1+dmas)/(xa*xa*xa))/asr;

    Vx = xn*((2*xp*xp - 1)*xms + 2*xp*xq*xmc);
    Vy = xn*((1 - 2*xq*xq)*xmc - 2*xp*xq*xms);
    Vz = 2*xn*xki*(xp*xms + xq*xmc);
}

void
EphemerisLow::calcHeliocentricXYZ(const double tjd, const double dmas,
                                  double *a, double *dlm, double *e, 
                                  double *pi, double *dinc, double *omega,
                                  int *kp, double *ca, double *sa, 
                                  int *kq, double *cl, double *sl,
                                  double &Px, double &Py, double &Pz,
                                  double &Vx, double &Vy, double &Vz)
{
    const double t = (tjd - 2451545)/365250;
    const double t2 = t*t;

    double da = a[0] + t*a[1] + t2*a[2];
    double dl = (3600*dlm[0] + t*dlm[1] + t2*dlm[2]) * deg_to_rad / 3600;
    const double de = e[0] + t*e[1] + t2*e[2];
    double dp = (3600*pi[0] + t*pi[1] + t2*pi[2]) * deg_to_rad / 3600;
    const double di = ((3600*dinc[0] + t*dinc[1] + t2*dinc[2]) 
                       * deg_to_rad / 3600);
    double dm = ((3600*omega[0] + t*omega[1] + t2*omega[2]) 
                 * deg_to_rad / 3600);

    const double dmu = 0.35953620*t;
    double arga, argl;
    for (int j = 0; j < 8; j++)
    {
        arga = kp[j] * dmu;
        da += (ca[j] * cos(arga) + sa[j] * sin(arga)) * 1e-7;

        argl = kq[j] * dmu;
        dl += (cl[j] * cos(argl) + sl[j] * sin(argl)) * 1e-7;
    }
    arga = kp[8] * dmu;
    da += t*(ca[8] * cos(arga) + sa[8] * sin(arga)) * 1e-7;
    for (int j = 8; j < 10; j++)
    {
        argl = kq[j] * dmu;
        dl += t*(cl[j] * cos(argl) + sl[j] * sin(argl)) * 1e-7;
    }
    dl = fmod(dl, TWO_PI);
    dp = fmod(dp, TWO_PI);
    dm = fmod(dm, TWO_PI);

    ellipx(da, dl, de, dp, di, dm, dmas, Px, Py, Pz, Vx, Vy, Vz);
}
