#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace std;

#include "elp82b.h"

#include "xpUtil.h"

void
moon(const double jd, double &X, double &Y, double &Z)
{
    const double sec_to_rad = deg_to_rad / 3600;

    double R[3] = { 0, 0, 0 };

    // w[0] is the mean mean longitude of the moon
    // w[1] is the mean longitude of the lunar perigee
    // w[2] is the mean longitude of the lunar ascending node
    // Units are seconds of arc
    double w[3][5];
    w[0][0] = (3600 * 218. + 60 * 18. + 59.95571);
    w[1][0] = (3600 *  83. + 60 * 21. + 11.67475);
    w[2][0] = (3600 * 125. + 60 *  2. + 40.39816);

    w[0][1] = 1732559343.73604;
    w[1][1] =   14643420.2632;
    w[2][1] =   -6967919.3622;

    w[0][2] =  -5.8883;
    w[1][2] = -38.2776;
    w[2][2] =   6.3622;

    w[0][3] =  0.6604e-2;
    w[1][3] = -0.45047e-1;
    w[2][3] =  0.7625e-2;

    w[0][4] = -0.3169e-4;
    w[1][4] =  0.21301e-3;
    w[2][4] = -0.3586e-4;

    // T and omega are angles of the inertial mean ecliptic of J2000
    // referred to the inertial mean equinox of J2000
    double T[5];
    T[0] = (3600 * 100. + 60 * 27. + 59.22059);
    T[1] = 129597742.2758;
    T[2] = -0.0202;
    T[3] = 0.9e-5;
    T[4] = 0.15e-6;

    double omega[5];
    omega[0] = (3600 * 102. + 60 * 56. + 14.42753);
    omega[1] = 1161.2283;
    omega[2] = 0.5327;
    omega[3] = -0.138e-3;
    omega[4] = 0;

    // precession between J2000 and the date
    double preces[5];
    preces[0] = 0;
    preces[1] = 5029.0966;
    preces[2] = 1.1120;
    preces[3] = .77e-4;
    preces[4] = -.2353e-4;

    // planetary longitudes in J2000
    double plon[8][2];
    plon[0][0] = (3600 * 252. + 60 * 15. + 3.25986);
    plon[1][0] = (3600 * 181. + 60 * 58. + 47.28305);
    plon[2][0] = T[0];
    plon[3][0] = (3600 * 355. + 60 * 25. + 59.78866);
    plon[4][0] = (3600 *  34. + 60 * 21. + 5.34212);
    plon[5][0] = (3600 *  50. + 60 *  4. + 38.89694);
    plon[6][0] = (3600 * 314. + 60 *  3. + 18.01841);
    plon[7][0] = (3600 * 304. + 60 * 20. + 55.19575);

    plon[0][1] = 538101628.68898;
    plon[1][1] = 210664136.43355;
    plon[2][1] = T[1];
    plon[3][1] = 68905077.59284;
    plon[4][1] = 10925660.42861;
    plon[5][1] = 4399609.65932;
    plon[6][1] = 1542481.19393;
    plon[7][1] = 786550.32074;

    // Convert to radians
    for (int i = 0; i < 8; i++)
    {
        plon[i][0] *= sec_to_rad;
        plon[i][1] *= sec_to_rad;
    }

    // Corrections of the constants (fit to DE200/LE200)
    double delnu =  0.55604 / w[0][1];
    double dele  =  0.01789 * sec_to_rad;
    double delg  = -0.08066 * sec_to_rad;
    double delnp = -0.06424 / w[0][1];
    double delep = -0.12879 * sec_to_rad;

    // Delaunay's arguments
    double del[4][5];
    for (int i = 0; i < 5; i++)
    {
        del[0][i] = w[0][i] - T[i];
        del[1][i] = T[i] - omega[i];
        del[2][i] = w[0][i] - w[1][i];
        del[3][i] = w[0][i] - w[2][i];
        for (int j = 0; j < 4; j++) 
            del[j][i] *= sec_to_rad;
    }
    del[0][0] += M_PI;

    double zeta[2];
    zeta[0] = w[0][0] * sec_to_rad;
    zeta[1] = (w[0][1] + preces[1]) * sec_to_rad;

    // precession matrix
    double p[5], q[5];
    p[0] =  0.10180391e-4;
    p[1] =  0.47020439e-6;
    p[2] = -0.5417367e-9;
    p[3] = -0.2507948e-11;
    p[4] =  0.463486e-14;
    q[0] = -0.113469002e-3;
    q[1] =  0.12372674e-6;
    q[2] =  0.1265417e-8;
    q[3] = -0.1371808e-11;
    q[4] = -0.320334e-14;
    
    double t[5];
    t[0] = 1;
    t[1] = (jd - 2451545.0) / 36525;
    t[2] = t[1] * t[1];
    t[3] = t[2] * t[1];
    t[4] = t[2] * t[2];

    double ath = 384747.9806743165;
    double a0 = 384747.9806448954; 
    double am = 0.074801329518;
    double alfa = 0.002571881335;
    double dtasm = 2 * alfa / (3 * am);

    // Main problem
    for (int i = 1; i < 4; i++)
    {
        int iv = ((i - 1) % 3);

        const int *ilu = ILU[i-1];
        const double *p_coef = COEF[i-1];

        for (int ii = 0; ii < NUM[i-1]; ii++)
        {
            if (ii > 0) 
            {
                ilu += 4;
                p_coef += 7;
            }

            double coef[7];
            for (int j = 0; j < 7; j++)
                coef[j] = p_coef[j];

            double x = coef[0];

            double tgv = coef[1] + dtasm * coef[5];

            if (i == 3) 
                coef[0] -= 2 * coef[0] * delnu / 3;

            x = (coef[0] + tgv * (delnp - am * delnu) 
                 + coef[2] * delg + coef[3] * dele 
                 + coef[4] * delep);
            double y = 0;

            for (int k = 0; k < 5; k++)
            {
                for (int j = 0; j < 4; j++)
                {
                    y += ilu[j] * del[j][k] * t[k];
                }
            }

            if (i == 3) y += M_PI_2;
            y = fmod(y, 2 * M_PI);

            R[iv] += x * sin(y);

        }
    }

    // Figures - Tides - Relativity - Solar eccentricity
    for (int i = 4; i < 37; i++)
    {
        if (i > 9 && i < 22) continue;
        int iv = ((i - 1) % 3);

        const int *ilu = ILU[i-1];
        const int *p_iz = IZ[i-1];
        const double *p_pha = PHA[i-1];
        const double *p_x = XX[i-1];

        for (int ii = 0; ii < NUM[i-1]; ii++)
        {
            if (ii > 0)
            {
                ilu += 4;
                p_iz++;
                p_pha++;
                p_x++;
            }
            
            int iz = *p_iz;
            double pha = *p_pha;
            double x = *p_x;

            if ((i > 6 && i < 10) || (i > 24 && i < 28)) x *= t[1];
            if (i > 33 && i < 37) x *= t[2];

            double y = pha * deg_to_rad;

            for (int k = 0; k < 2; k++)
            {
                y += iz * zeta[k] * t[k];
                for (int l = 0; l < 4; l++)
                {
                    y += ilu[l] * del[l][k] * t[k];
                }
            }
            y = fmod(y, 2*M_PI);
            R[iv] += x * sin(y);
        }
    }

    // Planetary perturbations
    for (int i = 10; i < 22; i++)
    {
        int iv = ((i - 1) % 3);

        const int *ipla = IPLA[i-1];
        const double *p_pha = PHA[i-1];
        const double *p_x = XX[i-1];

        for (int ii = 0; ii < NUM[i-1]; ii++)
        {
            if (ii > 0)
            {
                ipla += 11;
                p_pha++;
                p_x++;
            }

            double pha = *p_pha;
            double x = *p_x;

            if ((i > 12 && i < 16) || (i > 18 && i < 22)) x *= t[1];

            double y = pha * deg_to_rad;

            if (i < 16)
            {
                for (int k = 0; k < 2; k++)
                {
                    y += (ipla[8] * del[0][k] + ipla[9] * del[2][k] 
                          + ipla[10] * del[3][k]) * t[k];
                    for (int l = 0; l < 8; l++)
                    {
                        y += ipla[l] * plon[l][k] * t[k];
                    }
                }
            }
            else
            {
                for (int k = 0; k < 2; k++)
                {
                    for (int l = 0; l < 4; l++)
                        y += ipla[l+7] * del[l][k] * t[k];
                    for (int l = 0; l < 7; l++)
                    {
                        y += ipla[l] * plon[l][k] * t[k];
                    }
                }
            }

            y = fmod(y, 2*M_PI);
            R[iv] += x * sin(y);
        }
    }

    // Change of coordinates
    for (int i = 0; i < 5; i++)
        R[0] += w[0][i] * t[i];
    R[0] *= sec_to_rad;
    R[1] *= sec_to_rad;
    R[2] *= a0/ath;

    double x1 = R[2] * cos(R[1]);
    double x2 = x1 * sin(R[0]);
    x1 *= cos(R[0]);
    double x3 = R[2] * sin(R[1]);

    double pw = 0;
    double qw = 0;
    for (int i = 0; i < 5; i++)
    {
        pw += p[i] * t[i];
        qw += q[i] * t[i];
    }

    pw *= t[1];
    qw *= t[1];

    double ra = 2 * sqrt(1 - pw * pw - qw * qw);
    double pwqw = 2 * pw * qw;
    double pw2 = 1 - 2 * pw * pw;
    double qw2 = 1 - 2 * qw * qw;
    pw *= ra;
    qw *= ra;

    R[0] = pw2*x1+pwqw*x2+pw*x3;
    R[1] = pwqw*x1+qw2*x2-qw*x3;
    R[2] = -pw*x1+qw*x2+(pw2+qw2-1)*x3;

    X = R[0] / AU_to_km;
    Y = R[1] / AU_to_km;
    Z = R[2] / AU_to_km;

    // rotate to earth equator J2000
    const double eps = 23.4392911 * deg_to_rad;
    rotateX(X, Y, Z, -eps);
} 
