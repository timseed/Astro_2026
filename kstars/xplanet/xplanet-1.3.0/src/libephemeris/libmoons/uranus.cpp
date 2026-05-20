#include <cstdlib>
using namespace std;

#include "body.h"
#include "xpUtil.h"

/* 
  GUST86 ephemeris is described in Laskar & Jacobson,
  Astron. Astrophys. 188, 212-224 (1987)

  Much of this code is translated from the GUST86 FORTRAN code which
  is at ftp://ftp.bdl.fr/pub/ephem/satel/gust86
*/

static double
solveKepler(const double L, const double K, const double H)
{
    if (L == 0) return(0);

    double F = L;
    double E;

    double F0 = L;
    double E0 = fabs(L);

    const double eps = 1e-16;
    for (int i = 0; i < 20; i++)
    {
        const double SF = sin(F0);
        const double CF = cos(F0);
        const double FF0 = F0 - K*SF + H*CF - L;
        const double FPF0 = 1 - K*CF - H*SF;
        double SDIR = FF0/FPF0;

        double denom = 1;
        while (1)
        {
            F = F0 - SDIR / denom;
            E = fabs(F-F0);
            if (E <= E0) break;
            denom *= 2;
        }

        if (denom == 1 && E <= eps && FF0 <= eps) return(F);

        F0 = F;
        E0 = E;
    }
    return(F);
}

static void
calcRectangular(const double N, const double L, const double K, 
                const double H, const double Q, const double P, 
                const double GMS, 
                double &X, double &Y, double &Z)
{
    // Calculate the semi-major axis
    const double A = pow(GMS/(N*N), 1./3.) / AU_to_km;

    const double PHI = sqrt(1 - K*K - H*H);
    const double PSI = 1/(1+PHI);

    const double RKI = sqrt(1 - Q*Q - P*P);

    const double F = solveKepler(L, K, H);

    const double SF = sin(F);
    const double CF = cos(F);

    const double RLMF = -K*SF + H*CF;

    double rot[3][2];
    rot[0][0] = 1 - 2*P*P;
    rot[0][1] = 2*P*Q;
    rot[1][0] = 2*P*Q;
    rot[1][1] = 1 - 2*Q*Q;
    rot[2][0] = -2*P*RKI;
    rot[2][1] = 2*Q*RKI;

    double TX[2];
    TX[0] = A*(CF - PSI * H * RLMF - K);
    TX[1] = A*(SF + PSI * K * RLMF - H);

    X = rot[0][0] * TX[0] + rot[0][1] * TX[1];
    Y = rot[1][0] * TX[0] + rot[1][1] * TX[1];
    Z = rot[2][0] * TX[0] + rot[2][1] * TX[1];
}

// convert UME50* coordinates to EME50
static void
UranicentricToGeocentricEquatorial(double &X, double &Y, double &Z)
{
    const double alpha0 = 76.6067 * deg_to_rad;
    const double delta0 = 15.0322 * deg_to_rad;

    const double sa = sin(alpha0);
    const double sd = sin(delta0);
    const double ca = cos(alpha0);
    const double cd = cos(delta0);

    const double oldX = X;
    const double oldY = Y;
    const double oldZ = Z;

    X =  sa * oldX + ca * sd * oldY + ca * cd * oldZ;
    Y = -ca * oldX + sa * sd * oldY + sa * cd * oldZ;
    Z = -cd * oldY + sd * oldZ;
}

void
urasat(const double jd, const body b, 
       double &X, double &Y, double &Z)
{
    const double t = jd - 2444239.5;
    const double tcen = t/365.25;

    const double N1 = fmod(4.445190550 * t - 0.238051, TWO_PI);
    const double N2 = fmod(2.492952519 * t + 3.098046, TWO_PI);
    const double N3 = fmod(1.516148111 * t + 2.285402, TWO_PI);
    const double N4 = fmod(0.721718509 * t + 0.856359, TWO_PI);
    const double N5 = fmod(0.466692120 * t - 0.915592, TWO_PI);
    
    const double E1 = (20.082 * deg_to_rad * tcen + 0.611392);
    const double E2 = ( 6.217 * deg_to_rad * tcen + 2.408974);
    const double E3 = ( 2.865 * deg_to_rad * tcen + 2.067774);
    const double E4 = ( 2.078 * deg_to_rad * tcen + 0.735131);
    const double E5 = ( 0.386 * deg_to_rad * tcen + 0.426767);

    const double I1 = (-20.309 * deg_to_rad * tcen + 5.702313);
    const double I2 = ( -6.288 * deg_to_rad * tcen + 0.395757);
    const double I3 = ( -2.836 * deg_to_rad * tcen + 0.589326);
    const double I4 = ( -1.843 * deg_to_rad * tcen + 1.746237);
    const double I5 = ( -0.259 * deg_to_rad * tcen + 4.206896);

    const double GM1 = 4.4;
    const double GM2 = 86.1;
    const double GM3 = 84.0;
    const double GM4 = 230.0;
    const double GM5 = 200.0;
    const double GMU = 5794554.5 - (GM1 + GM2 + GM3 + GM4 + GM5);

    double N = 0, L = 0, K = 0, H = 0, Q = 0, P = 0, GMS = 0;

    switch (b)
    {
    case MIRANDA:
        N = (4443522.67 
             - 34.92 * cos(N1 - 3*N2 + 2*N3)
             +  8.47 * cos(2*N1 - 6*N2 + 4*N3)
             +  1.31 * cos(3*N1 - 9*N2 + 6*N3)
             - 52.28 * cos(N1 - N2)
             -136.65 * cos(2*N1 - 2*N2)) * 1e-6;

        L = (-238051.58
             + 4445190.55 * t
             + 25472.17 * sin(N1 - 3*N2 + 2*N3)
             -  3088.31 * sin(2*N1 - 6*N2 + 4*N3)
             -   318.10 * sin(3*N1 - 9*N2 + 6*N3)
             -    37.49 * sin(4*N1 - 12*N2 + 8*N3)
             -    57.85 * sin(N1 - N2)
             -    62.32 * sin(2*N1 - 2*N2)
             -    27.95 * sin(3*N1 - 3*N2)) * 1e-6;

        K = (1312.38 * cos(E1)
             + 71.81 * cos(E2)
             + 69.77 * cos(E3)
             +  6.75 * cos(E4)
             +  6.27 * cos(E5)
             - 123.31 * cos(-N1 + 2*N2)
             +  39.52 * cos(-2*N1 + 3*N2)
             + 194.10 * cos(N1)) * 1e-6;

        H = (1312.38 * sin(E1)
             + 71.81 * sin(E2)
             + 69.77 * sin(E3)
             +  6.75 * sin(E4)
             +  6.27 * sin(E5)
             - 123.31 * sin(-N1 + 2*N2)
             +  39.52 * sin(-2*N1 + 3*N2)
             + 194.10 * sin(N1)) * 1e-6;

        Q = (37871.71 * cos(I1)
             +  27.01 * cos(I2)
             +  30.76 * cos(I3)
             +  12.18 * cos(I4)
             +   5.37 * cos(I5)) * 1e-6;

        P = (37871.71 * sin(I1)
             +  27.01 * sin(I2)
             +  30.76 * sin(I3)
             +  12.18 * sin(I4)
             +   5.37 * sin(I5)) * 1e-6;

        GMS = GMU + GM1;
        break;
    case ARIEL:
        N = (2492542.57
             +   2.55 * cos(N1 - 3*N2 + 2*N3)
             -  42.16 * cos(N2 - N3)
             - 102.56 * cos(2*N2 - 2*N3)) * 1e-6;

        L = (3098046.41
             + 2492952.52 * t
             - 1860.50 * sin(N1 - 3*N2 + 2*N3)
             +  219.99 * sin(2*N1 - 6*N2 + 4*N3)
             +   23.10 * sin(3*N1 - 9*N2 + 6*N3)
             +    4.30 * sin(4*N1 - 12*N2 + 8*N3)
             -   90.11 * sin(N2 - N3)
             -   91.07 * sin(2*(N2 - N3))
             -   42.75 * sin(3*(N2 - N3))
             -   16.49 * sin(2*(N2 - N4))) * 1e-6;

        K = (-    3.35 * cos(E1)
             + 1187.63 * cos(E2)
             +  861.59 * cos(E3)
             +   71.50 * cos(E4)
             +   55.59 * cos(E5)
             -   84.60 * cos(-N2 + 2*N3)
             +   91.81 * cos(-2*N2 + 3*N3)
             +   20.03 * cos(-N2 + 2*N4)
             +   89.77 * cos(N2)) * 1e-6;

        H = (-    3.35 * sin(E1)
             + 1187.63 * sin(E2)
             +  861.59 * sin(E3)
             +   71.50 * sin(E4)
             +   55.59 * sin(E5)
             -   84.60 * sin(-N2 + 2*N3)
             +   91.81 * sin(-2*N2 + 3*N3)
             +   20.03 * sin(-N2 + 2*N4)
             +   89.77 * sin(N2)) * 1e-6;

        Q = (- 121.75 * cos(I1)
             + 358.25 * cos(I2)
             + 290.08 * cos(I3)
             +  97.78 * cos(I4)
             +  33.97 * cos(I5)) * 1e-6;

        P = (- 121.75 * sin(I1)
             + 358.25 * sin(I2)
             + 290.08 * sin(I3)
             +  97.78 * sin(I4)
             +  33.97 * sin(I5)) * 1e-6;

        GMS = GMU + GM2;
        break;
    case UMBRIEL:
        N = (1515954.90 
             +   9.74 * cos(N3 - 2*N4 + E3)
             - 106.00 * cos(N2 - N3)
             +  54.16 * cos(2*(N2 - N3))
             -  23.59 * cos(N3 - N4)
             -  70.70 * cos(2*(N3 - N4))
             -  36.28 * cos(3*(N3 - N4))) * 1e-6;

        L = (2285401.69
             + 1516148.11 * t
             + 660.57 * sin(  N1 - 3*N2 + 2*N3)
             -  76.51 * sin(2*N1 - 6*N2 + 4*N3)
             -   8.96 * sin(3*N1 - 9*N2 + 6*N3)
             -   2.53 * sin(4*N1 - 12*N2 + 8*N3)
             -  52.91 * sin(N3 - 4*N4 + 3*N5)
             -   7.34 * sin(N3 - 2*N4 + E5)
             -   1.83 * sin(N3 - 2*N4 + E4)
             + 147.91 * sin(N3 - 2*N4 + E3)
             -   7.77 * sin(N3 - 2*N4 + E2)
             +  97.76 * sin(N2 - N3)
             +  73.13 * sin(2*(N2 - N3))
             +  34.71 * sin(3*(N2 - N3))
             +  18.89 * sin(4*(N2 - N3))
             -  67.89 * sin(N3 - N4)
             -  82.86 * sin(2*(N3 - N4))
             -  33.81 * sin(3*(N3 - N4))
             -  15.79 * sin(4*(N3 - N4))
             -  10.21 * sin(N3 - N5)
             -  17.08 * sin(2*(N3 - N5))) * 1e-6;

        K = (-    0.21 * cos(E1)
             -  227.95 * cos(E2)
             + 3904.69 * cos(E3)
             +  309.17 * cos(E4)
             +  221.92 * cos(E5)
             +   29.34 * cos(N2)
             +   26.20 * cos(N3)
             +   51.19 * cos(-N2+2*N3)
             -  103.86 * cos(-2*N2+3*N3)
             -   27.16 * cos(-3*N2+4*N3)
             -   16.22 * cos(N4)
             +  549.23 * cos(-N3 + 2*N4)
             +   34.70 * cos(-2*N3 + 3*N4)
             +   12.81 * cos(-3*N3 + 4*N4)
             +   21.81 * cos(-N3 + 2*N5)
             +   46.25 * cos(N3)) * 1e-6;

        H = (-    0.21 * sin(E1)
             -  227.95 * sin(E2)
             + 3904.69 * sin(E3)
             +  309.17 * sin(E4)
             +  221.92 * sin(E5)
             +   29.34 * sin(N2)
             +   26.20 * sin(N3)
             +   51.19 * sin(-N2+2*N3)
             -  103.86 * sin(-2*N2+3*N3)
             -   27.16 * sin(-3*N2+4*N3)
             -   16.22 * sin(N4)
             +  549.23 * sin(-N3 + 2*N4)
             +   34.70 * sin(-2*N3 + 3*N4)
             +   12.81 * sin(-3*N3 + 4*N4)
             +   21.81 * sin(-N3 + 2*N5)
             +   46.25 * sin(N3)) * 1e-6;

        Q = (-   10.86 * cos(I1)
             -   81.51 * cos(I2)
             + 1113.36 * cos(I3)
             +  350.14 * cos(I4)
             +  106.50 * cos(I5)) * 1e-6;

        P = (-   10.86 * sin(I1)
             -   81.51 * sin(I2)
             + 1113.36 * sin(I3)
             +  350.14 * sin(I4)
             +  106.50 * sin(I5)) * 1e-6;

        GMS = GMU + GM3;
        break;
    case TITANIA:
        N = (721663.16
             -  2.64 * cos(N3 - 2*N4 + E3)
             -  2.16 * cos(2*N4 - 3*N5 + E5)
             +  6.45 * cos(2*N4 - 3*N5 + E4)
             -  1.11 * cos(2*N4 - 3*N5 + E3)
             - 62.23 * cos(N2 - N4)
             - 56.13 * cos(N3 - N4)
             - 39.94 * cos(N4 - N5)
             - 91.85 * cos(2*(N4 - N5))
             - 58.31 * cos(3*(N4 - N5))
             - 38.60 * cos(4*(N4 - N5))
             - 26.18 * cos(5*(N4 - N5))
             - 18.06 * cos(6*(N4 - N5))) * 1e-6;

        L = (856358.79
             + 721718.51 * t
             +  20.61 * sin(N3 - 4*N4 + 3*N5)
             -   2.07 * sin(N3 - 2*N4 + E5)
             -   2.88 * sin(N3 - 2*N4 + E4)
             -  40.79 * sin(N3 - 2*N4 + E3)
             +   2.11 * sin(N3 - 2*N4 + E2)
             -  51.83 * sin(2*N4 - 3*N5 + E5)
             + 159.87 * sin(2*N4 - 3*N5 + E4)
             -  35.05 * sin(2*N4 - 3*N5 + E3)
             -   1.56 * sin(3*N4 - 4*N5 + E5)
             +  40.54 * sin(N2 - N4)
             +  46.17 * sin(N3 - N4)
             - 317.76 * sin(N4 - N5)
             - 305.59 * sin(2*(N4 - N5))
             - 148.36 * sin(3*(N4 - N5))
             -  82.92 * sin(4*(N4 - N5))
             -  49.98 * sin(5*(N4 - N5))
             -  31.56 * sin(6*(N4 - N5))
             -  20.56 * sin(7*(N4 - N5))
             -  13.69 * sin(8*(N4 - N5))) * 1e-6;

        K = (-    0.02 * cos(E1)
             -    1.29 * cos(E2)
             -  324.51 * cos(E3)
             +  932.81 * cos(E4)
             + 1120.89 * cos(E5)
             +   33.86 * cos(N2)
             +   17.46 * cos(N4)
             +   16.58 * cos(-N2 + 2*N4)
             +   28.89 * cos(N3)
             -   35.86 * cos(-N3 + 2*N4)
             -   17.86 * cos(N4)
             -   32.10 * cos(N5)
             -  177.83 * cos(-N4 + 2*N5)
             +  793.43 * cos(-2*N4 + 3*N5)
             +   99.48 * cos(-3*N4 + 4*N5)
             +   44.83 * cos(-4*N4 + 5*N5)
             +   25.13 * cos(-5*N4 + 6*N5)
             +   15.43 * cos(-6*N4 + 7*N5)) * 1e-6;

        H = (-    0.02 * sin(E1)
             -    1.29 * sin(E2)
             -  324.51 * sin(E3)
             +  932.81 * sin(E4)
             + 1120.89 * sin(E5)
             +   33.86 * sin(N2)
             +   17.46 * sin(N4)
             +   16.58 * sin(-N2 + 2*N4)
             +   28.89 * sin(N3)
             -   35.86 * sin(-N3 + 2*N4)
             -   17.86 * sin(N4)
             -   32.10 * sin(N5)
             -  177.83 * sin(-N4 + 2*N5)
             +  793.43 * sin(-2*N4 + 3*N5)
             +   99.48 * sin(-3*N4 + 4*N5)
             +   44.83 * sin(-4*N4 + 5*N5)
             +   25.13 * sin(-5*N4 + 6*N5)
             +   15.43 * sin(-6*N4 + 7*N5)) * 1e-6;

        Q = (-   1.43 * cos(I1)
             -   1.06 * cos(I2)
             - 140.13 * cos(I3)
             + 685.72 * cos(I4)
             + 378.32 * cos(I5)) * 1e-6;
             
        P = (-   1.43 * sin(I1)
             -   1.06 * sin(I2)
             - 140.13 * sin(I3)
             + 685.72 * sin(I4)
             + 378.32 * sin(I5)) * 1e-6;
             
        GMS = GMU + GM4;
        break;
    case OBERON:
        N = (466580.54
             +  2.08 * cos(2*N4 - 3*N5 + E5)
             -  6.22 * cos(2*N4 - 3*N5 + E4)
             +  1.07 * cos(2*N4 - 3*N5 + E3)
             - 43.10 * cos(N2 - N5)
             - 38.94 * cos(N3 - N5)
             - 80.11 * cos(N4 - N5)
             + 59.06 * cos(2*(N4 - N5))
             + 37.49 * cos(3*(N4 - N5))
             + 24.82 * cos(4*(N4 - N5))
             + 16.84 * cos(5*(N4 - N5))) * 1e-6;

        L = (-915591.80
             + 466692.12 * t
             -   7.82 * sin(N3 - 4*N4 + 3*N5)
             +  51.29 * sin(2*N4 - 3*N5 + E5)
             - 158.24 * sin(2*N4 - 3*N5 + E4)
             +  34.51 * sin(2*N4 - 3*N5 + E3)
             +  47.51 * sin(N2 - N5)
             +  38.96 * sin(N3 - N5)
             + 359.73 * sin(N4 - N5)
             + 282.78 * sin(2*(N4 - N5))
             + 138.60 * sin(3*(N4 - N5))
             +  78.03 * sin(4*(N4 - N5))
             +  47.29 * sin(5*(N4 - N5))
             +  30.00 * sin(6*(N4 - N5))
             +  19.62 * sin(7*(N4 - N5))
             +  13.11 * sin(8*(N4 - N5))) * 1e-6;

        K = (        0 * cos(E1)
             -    0.35 * cos(E2)
             +   74.53 * cos(E3)
             -  758.68 * cos(E4)
             + 1397.34 * cos(E5)
             +   39.00 * cos(N2)
             +   17.66 * cos(-N2 + 2*N5)
             +   32.42 * cos(N3)
             +   79.75 * cos(N4)
             +   75.66 * cos(N5)
             +  134.04 * cos(-N4 + 2*N5)
             -  987.26 * cos(-2*N4 + 3*N5)
             -  126.09 * cos(-3*N4 + 4*N5)
             -   57.42 * cos(-4*N4 + 5*N5)
             -   32.41 * cos(-5*N4 + 6*N5)
             -   19.99 * cos(-6*N4 + 7*N5)
             -   12.94 * cos(-7*N4 + 8*N5)) * 1e-6;

        H = (0 * sin(E1)
             -    0.35 * sin(E2)
             +   74.53 * sin(E3)
             -  758.68 * sin(E4)
             + 1397.34 * sin(E5)
             +   39.00 * sin(N2)
             +   17.66 * sin(-N2 + 2*N5)
             +   32.42 * sin(N3)
             +   79.75 * sin(N4)
             +   75.66 * sin(N5)
             +  134.04 * sin(-N4 + 2*N5)
             -  987.26 * sin(-2*N4 + 3*N5)
             -  126.09 * sin(-3*N4 + 4*N5)
             -   57.42 * sin(-4*N4 + 5*N5)
             -   32.41 * sin(-5*N4 + 6*N5)
             -   19.99 * sin(-6*N4 + 7*N5)
             -   12.94 * sin(-7*N4 + 8*N5)) * 1e-6;

        Q = (-   0.44 * cos(I1)
             -   0.31 * cos(I2)
             +  36.89 * cos(I3)
             - 596.33 * cos(I4)
             + 451.69 * cos(I5)) * 1e-6;

        P = (-   0.44 * sin(I1)
             -   0.31 * sin(I2)
             +  36.89 * sin(I3)
             - 596.33 * sin(I4)
             + 451.69 * sin(I5)) * 1e-6;

        GMS = GMU + GM5;
    break;
    default:
        xpExit("Unknown Uranus satellite\n", __FILE__, __LINE__);
    }

    X = 0; 
    Y = 0;
    Z = 0;

    N /= 86400;
    L = fmod(L, TWO_PI);

    calcRectangular(N, L, K, H, Q, P, GMS, X, Y, Z);

    UranicentricToGeocentricEquatorial(X, Y, Z);

    // precess to J2000
    precessB1950J2000(X, Y, Z);
}
