#include <cstdio>
#include <cstdlib>
using namespace std;

#include "body.h"
#include "xpUtil.h"

/*
  The Galilean satellite ephemerides E5 are described in Lieske,
  Astron. Astrophys. Suppl. Ser., 129, 205-217 (1998)
*/

static void
computeArguments(const double t, 
                 double &l1, double &l2, double &l3, double &l4,
                 double &om1, double &om2, double &om3, double &om4,
                 double &psi, double &Gp, double &G)
{
    // mean longitudes
    l1 = (106.077187 + 203.48895579033 * t) * deg_to_rad;
    l2 = (175.731615 + 101.37472473479 * t) * deg_to_rad;
    l3 = (120.558829 +  50.31760920702 * t) * deg_to_rad;
    l4 = ( 84.444587 +  21.57107117668 * t) * deg_to_rad;

    // proper nodes
    om1 = (312.334566 - 0.13279385940 * t) * deg_to_rad;
    om2 = (100.441116 - 0.03263063731 * t) * deg_to_rad;
    om3 = (119.194241 - 0.00717703155 * t) * deg_to_rad;
    om4 = (322.618633 - 0.00175933880 * t) * deg_to_rad;

    // longitude of origin of coordingate (Jupiter's pole)
    psi = (316.518203 - 2.08362E-06 * t) * deg_to_rad;

    // mean anomaly of Saturn
    Gp = (31.978528 + 0.03345973390 * t) * deg_to_rad;

    // mean anomaly of Jupiter
    G = (30.237557 + 0.08309257010 * t) * deg_to_rad;
}

void
jupsat(const double jd, const body b, 
       double &X, double &Y, double &Z)
{
    const double t = jd - 2443000.5;

    // mean longitudes
    double l1, l2, l3, l4;

    // proper nodes
    double om1, om2, om3, om4;

    // longitude of origin of coordinates (Jupiter's pole)
    double psi;

    // mean anomaly of Saturn
    double Gp;

    // mean anomaly of Jupiter
    double G;

    computeArguments(t, l1, l2, l3, l4, om1, om2, om3, om4, psi, Gp, G);
    
    // free libration
    const double phi = (199.676608 + 0.17379190461 * t) * deg_to_rad;

    // periapse longitudes
    const double pi1 = ( 97.088086 + 0.16138586144 * t) * deg_to_rad;
    const double pi2 = (154.866335 + 0.04726306609 * t) * deg_to_rad;
    const double pi3 = (188.184037 + 0.00712733949 * t) * deg_to_rad;
    const double pi4 = (335.286807 + 0.00183999637 * t) * deg_to_rad;

    // longitude of perihelion of jupiter
    const double PIj = 13.469942 * deg_to_rad;
    
    // phase angles
    const double phi1 = 188.374346 * deg_to_rad;
    const double phi2 =  52.224824 * deg_to_rad;
    const double phi3 = 257.184000 * deg_to_rad;
    const double phi4 = 149.152605 * deg_to_rad;

    // semimajor axes, in AU
    const double axis1 =  2.819353E-3;
    const double axis2 =  4.485883E-3;
    const double axis3 =  7.155366E-3;
    const double axis4 = 12.585464E-3;

    // common factors
    const double PIG2 = (PIj + G) * 2;

    double xi = 0;
    double upsilon = 0;
    double zeta = 0;
    double radius, lon;

    switch (b)
    {
    case IO:
    {
        lon = l1;

        const int NXI = 10;
        double XI[NXI][2] = { 
            {    170, l1 - l2 },
            {    106, l1 - l3 },
            {     -2, l1 - pi1 },
            {     -2, l1 - pi2 },
            {   -387, l1 - pi3 },
            {   -214, l1 - pi4 },
            {    -66, l1 + pi3 - PIG2 },
            { -41339, 2*(l1 - l2) },
            {      3, 2*(l1 - l3) },
            {   -131, 4*(l1-l2) }
        };
        
        for (int i = 0; i < NXI; i++)
            xi += XI[i][0] * cos(XI[i][1]);
        xi *= 1e-7;

        radius = axis1 * (1 + xi);

        const int NU = 41;
        double U[NU][2] = {
            {   -26, 2 * psi - PIG2 },
            {  -553, 2*(psi - PIj) },
            {  -240, om3 + psi - PIG2 },
            {    92, psi - om2 },
            {   -72, psi - om3 },
            {   -49, psi - om4 },
            {  -325, G },
            {    65, 2*G },
            {   -33, 5*Gp - 2*G + phi2 },
            {   -27, om3 - om4 },
            {   145, om2 - om3 },
            {    30, om2 - om4 },
            {   -38, pi4 - PIj },
            { -6071, pi3 - pi4 },
            {   282, pi2 - pi3 },
            {   156, pi2 - pi4 },
            {   -38, pi1 - pi3 },
            {   -25, pi1 - pi4 },
            {   -27, pi1 + pi4 - PIG2 },
            { -1176, pi1 + pi3 - PIG2 },
            {  1288, phi },
            {    39, 3*l3 - 7*l4 + 4*pi4 },
            {   -32, 3*l3 - 7*l4 + pi3 + 3*pi4 },
            { -1162, l1 - 2*l2 + pi4 },
            { -1887, l1 - 2*l2 + pi3 },
            { -1244, l1 - 2*l2 + pi2 },
            {    38, l1 - 2*l2 + pi1 },
            {  -617, l1 - l2 },
            {  -270, l1 - l3 },
            {   -26, l1 - l4 },
            {     4, l1 - pi1 },
            {     5, l1 - pi2 },
            {   776, l1 - pi3 },
            {   462, l1 - pi4 },
            {   149, l1 + pi3 - PIG2 },
            {    21, 2*l1 - 4*l2 + om2 + om3 },
            {  -200, 2*l1 - 4*l2 + 2*om2 },
            { 82483, 2*(l1 - l2) },
            {   -35, 2*(l1 - l3) },
            {    -3, 3*l1 - 4*l2 + pi3 },
            {   276, 4*(l1 - l2) }
        };

        for (int i = 0; i < NU; i++)
            upsilon += U[i][0] * sin(U[i][1]);
        upsilon *= 1e-7;

        // now use the "time completed" series
        const double n = 203.48895579033 * deg_to_rad;
        computeArguments(t + upsilon/n, l1, l2, l3, l4,
                         om1, om2, om3, om4, psi, Gp, G);

        const int NZ = 7;
        double Z[NZ][2] = {
            {   46, l1 + psi - 2*PIj - 2*G},
            { 6393, l1 - om1 },
            { 1825, l1 - om2 },
            {  329, l1 - om3 },
            {   93, l1 - om4 },
            { -311, l1 - psi },
            {   75, 3*l1 - 4*l2 + om2 }
        };

        for (int i = 0; i < NZ; i++)
            zeta += Z[i][0] * sin(Z[i][1]);
        zeta *= 1e-7;
        
    }
    break;
    case EUROPA:
    {
        lon = l2;

        const int NXI = 24;
        double XI[NXI][2] = { 
            {   -18, om2 - om3 },
            {   -27, 2*l3 - PIG2 },
            {   553, l2 - l3 },
            {    45, l2 - l4 },
            {  -102, l2 - pi1 },
            { -1442, l2 - pi2 },
            { -3116, l2 - pi3 },
            { -1744, l2 - pi4 },
            {   -15, l2 - PIj - G },
            {   -64, 2*(l2 - l4) },
            {   164, 2*(l2 - om2) },
            {    18, 2*l2 - om2 - om3 },
            {   -54, 5*(l2 - l3) },
            {   -30, l1 - 2*l2 + pi4 },
            {   -67, l1 - 2*l2 + pi3 },
            { 93848, l1 - l2 },
            {    48, l1 - 2*l3 + pi4 },
            {   107, l1 - 2*l3 + pi3 },
            {   -19, l1 - 2*l3 + pi2 },
            {   523, l1 - l3 },
            {    30, l1 - pi3 },
            {  -290, 2*(l1 - l2) },
            {   -91, 2*(l1 - l3) },
            {    22, 4*(l1 - l2) }
        };

        for (int i = 0; i < NXI; i++)
            xi += XI[i][0] * cos(XI[i][1]);
        xi *= 1e-7;

        radius = axis2 * (1 + xi);

        const int NU = 66;
        double U[NU][2] = {
            {      98, 2*psi - PIG2 },
            {   -1353, 2*(psi - PIj) },
            {     551, psi + om3 - PIG2 },
            {      26, psi + om2 - PIG2 },
            {      31, psi - om2 },
            {     255, psi - om3 },
            {     218, psi - om4 },
            {   -1845, G},
            {    -253, 2*G },
            {      18, 2*(Gp - G) + phi4 },
            {      19, 2*Gp - G + phi1 },
            {     -15, 5*Gp - 3*G + phi1 },
            {    -150, 5*G - 2*G + phi2 },
            {     102, om3 - om4 },
            {      56, om2 - om3 },
            {      72, pi4 - PIj },
            {    2259, pi3 - pi4 },
            {     -24, pi3 - pi4 + om3 - om4 },
            {     -23, pi2 - pi3 },
            {     -36, pi2 - pi4 },
            {     -31, pi1 - pi2 },
            {       4, pi1 - pi3 },
            {     111, pi1 - pi4 },
            {    -354, pi1 + pi3 - PIG2 },
            {   -3103, phi },
            {      55, 2*l3 - PIG2 },
            {    -111, 3*l3 - 7*l4 + 4*pi4 },
            {      91, 3*l3 - 7*l4 + pi3 + 3*pi4 },
            {     -25, 3*l3 - 7*l4 + 2*pi3 + 2*pi4 },
            {   -1994, l2 - l3 },
            {    -137, l2 - l4 },
            {       1, l2 - pi1 },
            {    2886, l2 - pi2 },
            {    6250, l2 - pi3 },
            {    3463, l2 - pi4 },
            {      30, l2 - PIj - G },
            {     -18, 2*l2 - 3*l3 + pi4 },
            {     -39, 2*l2 - 3*l3 + pi3 },
            {      98, 2*(l2 - l4) },
            {    -164, 2*(l2 - om2) },
            {     -18, 2*l2 - om2 - om3 },
            {      72, 5*(l2 - l3) },
            {      30, l1 - 2*l2 - pi3 + PIG2 },
            {    4180, l1 - 2*l2 + pi4, },
            {    7428, l1 - 2*l2 + pi3, },
            {   -2329, l1 - 2*l2 + pi2, },
            {     -19, l1 - 2*l2 + pi1, },
            { -185835, l1 - l2 },
            {    -110, l1 - 2*l3 + pi4, },
            {    -200, l1 - 2*l3 + pi3, },
            {      39, l1 - 2*l3 + pi2, },
            {     -16, l1 - 2*l3 + pi1, },
            {    -803, l1 - l3 },
            {     -19, l1 - pi2 },
            {     -75, l1 - pi3 },
            {     -31, l1 - pi4 },
            {      -9, 2*l1 - 4*l2 + om3 + psi },
            {       4, 2*l1 - 4*l2 + 2*om3 },
            {     -14, 2*l1 - 4*l2 + om2 + om3 },
            {     150, 2*l1 - 4*l2 + 2*om2 },
            {     -11, 2*l1 - 4*l2 + PIG2 },
            {      -9, 2*l1 - 4*l2 + pi3 + pi4, },
            {      -8, 2*l1 - 4*l2 + 2*pi3 },
            {     915, 2*(l1 - l2) },
            {      96, 2*(l1 - l3) },
            {     -18, 4*(l1 - l2) }
        };

        for (int i = 0; i < NU; i++)
            upsilon += U[i][0] * sin(U[i][1]);
        upsilon *= 1e-7;

        // now use the "time completed" series
        const double n = 101.37472473479 * deg_to_rad;
        computeArguments(t + upsilon/n, l1, l2, l3, l4,
                         om1, om2, om3, om4, psi, Gp, G);

        const int NZ = 11;
        double Z[NZ][2] = {
            {    17, l2 + psi - 2*(PIj - G) - G },
            {   143, l2 + psi - 2*(PIj - G) },
            {  -144, l2 - om1 },
            { 81004, l2 - om2 },
            {  4512, l2 - om3 },
            {  1160, l2 - om4 },
            {   -19, l2 - psi - G },
            { -3284, l2 - psi },
            {    35, l2 - psi + G },
            {   -28, l1 - 2*l3 + om3 },
            {   272, l1 - 2*l3 + om2 }
        };

        for (int i = 0; i < NZ; i++)
            zeta += Z[i][0] * sin(Z[i][1]);
        zeta *= 1e-7;
    }
    break;
    case GANYMEDE:
    {
        lon = l3;

        const int NXI = 31;
        double XI[NXI][2] = { 
            {     24, psi - om3 },
            {     -9, om3 - om4 },
            {     10, pi3 - pi4 },
            {    294, l3 - l4 },
            {     18, l3 - pi2 },
            { -14388, l3 - pi3 },
            {  -7919, l3 - pi4 },
            {    -23, l3 - PIj - G },
            {    -20, l3 + pi4 - PIG2 },
            {    -51, l3 + pi3 - PIG2 },
            {     39, 2*l3 - 3*l4 + pi4 },
            {  -1761, 2*(l3 - l4) },
            {    -11, 2*(l3 - pi3) },
            {    -10, 2*(l3 - pi3 - pi4) },
            {    -27, 2*l3 - PIG2 },
            {     24, 2*(l3 - om3) },
            {      9, 2 * l3 - om3 - om4 },
            {    -24, 2 * l3 - om3 - psi },
            {    -16, 3*l3 - 4*l4 + pi4 },
            {   -156, 3*(l3 - l4) },
            {    -42, 4*(l3 - l4) },
            {    -11, 5*(l3 - l4) },
            {   6342, l2 - l3 },
            {      9, l2 - pi3 },
            {     39, 2*l2 - 3*l3 + pi4 },
            {     70, 2*l2 - 3*l3 + pi3 },
            {     10, l1 - 2*l2 + pi4 },
            {     20, l1 - 2*l2 + pi3 },
            {   -153, l1 - l2 },
            {    156, l1 - l3 },
            {     11, 2*(l1 - l2) }
        };

        for (int i = 0; i < NXI; i++)
            xi += XI[i][0] * cos(XI[i][1]);
        xi *= 1e-7;

        radius = axis3 * (1 + xi);

        const int NU = 75;
        double U[NU][2] = {
            {     10, psi - pi3 + pi4 - om3 },
            {     28, 2*psi - PIG2 },
            {  -1770, 2*(psi - PIj) },
            {    -48, psi + om3 - PIG2 },
            {     14, psi - om2 },
            {    411, psi - om3 },
            {    345, psi - om4 },
            {  -2338, G },
            {    -66, 2*G },
            {     10, Gp - G + phi3 },
            {     22, 2*(Gp - G) + phi4 },
            {     26, 2*Gp - G + phi1 },
            {     11, 3*Gp - 2*G + phi2 + phi3 },
            {      9,  3*Gp - G + phi1 - phi2 },
            {    -19, 5*Gp - 3*G + phi1 },
            {   -208, 5*Gp - 2*G + phi2 },
            {    159, om3 - om4 },
            {     21, om2 - om3 },
            {    121, pi4 - PIj },
            {   6604, pi3 - pi4 },
            {    -65, pi3 - pi4 + om3 - om4 },
            {    -88, pi2 - pi3 },
            {    -72, pi2 - pi4 },
            {    -26, pi1 - pi3 },
            {     -9, pi1 - pi4 },
            {     16, pi1 + pi4 - PIG2 },
            {    125, pi1 + pi3 - PIG2 },
            {    307, phi },
            {    -10, l4 - pi4 },
            {   -100, l3 - 2*l4 + pi4 },
            {     83, l3 - 2*l4 + pi3 },
            {   -944, l3 - l4 },
            {    -37, l3 - pi2 },
            {  28780, l3 - pi3 },
            {  15849, l3 - pi4 },
            {      7, l3 - pi4 + om3 - om4 },
            {     46, l3 - PIj - G },
            {     51, l3 + pi4 - PIG2 },
            {     11, l3 + pi3 - PIG2 - G },
            {     97, l3 + pi3 - PIG2 },
            {      1, l3 + pi1 - PIG2 },
            {   -101, 2*l3 - 3*l4 + pi4 },
            {     13, 2*l3 - 3*l4 + pi3 },
            {   3222, 2*(l3 - l4) },
            {     29, 2*(l3 - pi3) },
            {     25, 2*l3 - pi3 - pi4 },
            {     37, 2*l3 - PIG2 },
            {    -24, 2*(l3 - om3) },
            {     -9, 2*l3 - om3 - om4 },
            {     24, 2*l3 - om3 - psi },
            {   -174, 3*l3 - 7*l4 + 4*pi4 },
            {    140, 3*l3 - 7*l4 + pi3 + 3*pi4 },
            {    -55, 3*l3 - 7*l4 + 2*pi3 + 2*pi4 },
            {     27, 3*l3 - 4*l4 + pi4 },
            {    227, 3*(l3 - l4) },
            {     53, 4*(l3 - l4) },
            {     13, 5*(l3 - l4) },
            {     42, l2 - 3*l3 + 2*l4 },
            { -12055, l2 - l3 },
            {    -24, l2 - pi3 },
            {    -10, l2 - pi4 },
            {    -79, 2*l2 - 3*l3 + pi4 },
            {   -131, 2*l2 - 3*l3 + pi3 },
            {   -665, l1 - 2*l2 + pi4 }, 
            {  -1228, l1 - 2*l2 + pi3 }, 
            {   1082, l1 - 2*l2 + pi2 }, 
            {     90, l1 - 2*l2 + pi1 }, 
            {    190, l1 - l2 },
            {    218, l1 - l3 },
            {      2, 2*l1 - 4*l2 + om3 + psi },
            {     -4, 2*l1 - 4*l2 + 2*om3 },
            {      3, 2*l1 - 4*l2 + 2*om2 },
            {      2, 2*l1 - 4*l2 + pi3 + pi4 },
            {      2, 2*l1 - 4*l2 + 2*pi3 },
            {    -13, 2*(l1 - l2) }
        };
        
        for (int i = 0; i < NU; i++)
            upsilon += U[i][0] * sin(U[i][1]);
        upsilon *= 1e-7;

        // now use the "time completed" series
        const double n = 50.31760920702 * deg_to_rad;
        computeArguments(t + upsilon/n, l1, l2, l3, l4, 
                         om1, om2, om3, om4, psi, Gp, G);

        const int NZ = 13;
        double Z[NZ][2] = {
            {     37, l2 + psi - 2*(PIj - G) - G },
            {    321, l2 + psi - 2*(PIj - G) },
            {    -15, l2 + psi - 2*PIj - G },
            {    -45, l3 - 2*PIj + psi },
            {  -2797, l3 - om2 },
            {  32402, l3 - om3 },
            {   6847, l3 - om4 },
            {    -45, l3 - psi - G },
            { -16911, l3 - psi },
            {     51, l3 - psi + G },
            {     10, 2*l2 - 3*l3 + psi },
            {    -21, 2*l2 - 3*l3 + om3 },
            {     30, 2*l2 - 3*l3 + om2 }
        };

        for (int i = 0; i < NZ; i++)
            zeta += Z[i][0] * sin(Z[i][1]);
        zeta *= 1e-7;
    }
    break;
    case CALLISTO:
    {
        lon = l4;

        const int NXI = 43;
        double XI[NXI][2] = { 
            {    -19, psi - om3 },
            {    167, psi - om4 },
            {     11, G },
            {     12, om3 - om4 },
            {    -13, pi3 - pi4 },
            {   1621, l4 - pi3 },
            {    -24, l4 - pi4 + 2*(psi - PIj) },
            {    -17, l4 - pi4 - G },
            { -73546, l4 - pi4 },
            {     15, l4 - pi4 + G },
            {     30, l4 - pi4 + 2*(PIj - psi) },
            {     -5, l4 - PIj + 2*G },
            {    -89, l4 - PIj - G },
            {    182, l4 - PIj },
            {     -6, l4 + pi4 - 2*PIj - 4*G },
            {    -62, l4 + pi4 - 2*PIj - 3*G },
            {   -543, l4 + pi4 - 2*PIj - 2*G },
            {     27, l4 + pi4 - 2*PIj - G },
            {      6, l4 + pi4 - 2*PIj },
            {      6, l4 + pi4 - om4 - psi },
            {     -9, l4 + pi3 - 2*pi4 },
            {     14, l4 + pi3 - PIG2 },
            {     13, 2*l4 - pi3 - pi4 },
            {   -271, 2*(l4 - pi4) },
            {    -25, 2*l4 - PIG2 - G },
            {   -155, 2*l4 - PIG2 },
            {    -12, 2*l4 - om3 - om4 },
            {     19, 2*l4 - om3 - psi },
            {     48, 2*(l4 - om4) },
            {   -167, 2*l4 - om4 - psi },
            {    142, 2*(l4 - psi) },
            {    -22, l3 - 2*l4 + pi4 },
            {     20, l3 - 2*l4 + pi3 },
            {    974, l3 - l4 },
            {     24, 2*l3 - 3*l4 + pi4 },
            {    177, 2*(l3 - l4) },
            {      4, 3*l3 - 4*l4 + pi4 },
            {     42, 3*(l3 - l4) },
            {     14, 4*(l3 - l4) },
            {      5, 5*(l3 - l4) },
            {     -8, l2 - 3*l3 + 2*l4 },
            {     92, l2 - l4 },
            {    105, l1 - l4 }
        };

        for (int i = 0; i < NXI; i++)
            xi += XI[i][0] * cos(XI[i][1]);
        xi *= 1e-7;

        radius = axis4 * (1 + xi);

        const int NU = 86;
        double U[NU][2] = {
            {      8, 2*psi - pi3 - pi4 },
            {     -9, psi - pi3 - pi4 + om4 },
            {     27, psi - pi3 + pi4 - om4 },
            {   -409, 2*(psi - pi4) },
            {    310, psi - 2*pi4 + om4 },
            {    -19, psi - 2*pi4 + om3 },
            {      8, 2*psi - pi4 - PIj },
            {     -5, psi - pi4 - PIj + om4 },
            {     63, psi - pi4 + PIj - om4 },
            {      8, 2*psi - PIG2 - G },
            {     73, 2*psi - PIG2 },
            {  -5768, 2*(psi - PIj) },
            {     16, psi + om4 - PIG2 },
            {    -97, psi - om3 },
            {    152, 2*(psi - om4) },
            {   2070, psi - om4 },
            {  -5604, G },
            {   -204, 2*G },
            {    -10, 3*G },
            {     24, Gp - G + phi3 },
            {     11, Gp + phi1 - 2*phi2 },
            {     52, 2*(Gp - G) + phi4 },
            {     61, 2*Gp - G + phi1 },
            {     25, 3*Gp - 2*G + phi2 + phi3 },
            {     21, 3*Gp - G + phi1 - phi2 },
            {    -45, 5*Gp - 3*G + phi1 },
            {   -495, 5*Gp - 3*G + phi2 },
            {    -44, om3 - om4 },
            {      5, pi4 - PIj - G },
            {    234, pi4 - PIj },
            {     11, 2*pi4 - PIG2 },
            {    -10, 2*pi4 - om3 - om4 },
            {     68, 2*(pi4 - om4) },
            {    -13, pi3 - pi4 - om4 + psi },
            {  -5988, pi3 - pi4 },
            {    -47, pi3 - pi4 + om3 - om4 },
            {  -3249, l4 - pi3 },
            {     48, l4 - pi4 + 2*(psi - PIj) },
            {     10, l4 - pi4 - om4 + psi },
            {     33, l4 - pi4 - G },
            { 147108, l4 - pi4 },
            {    -31, l4 - pi4 + G },
            {     -6, l4 - pi4 + om4 - psi },
            {    -61, l4 - pi4 + 2*(PIj - psi) },
            {     10, l4 - PIj - 2*G },
            {    178, l4 - PIj - G },
            {   -363, l4 - PIj },
            {      5, l4 + pi4 - 2*PIj - 5*Gp + 2*G - phi1 },
            {     12, l4 + pi4 - 2*PIj - 4*G },
            {    124, l4 + pi4 - 2*PIj - 3*G },
            {   1088, l4 + pi4 - 2*PIj - 2*G },
            {    -55, l4 + pi4 - 2*PIj - G },
            {    -12, l4 + pi4 - 2*PIj },
            {    -13, l4 + pi4 - om4 - psi },
            {      6, l4 + pi4 - 2*psi },
            {     17, l4 + pi3 - 2*pi4 },
            {    -28, l4 + pi3 - PIG2 },
            {    -33, 2*l4 - pi3 - pi4 },
            {    676, 2*(l4 - pi4) },
            {     36, 2*(l4 - PIj - G) - G },
            {    218, 2*(l4 - PIj - G) },
            {     -5, 2*(l4 - PIj) - G },
            {     12, 2*l4 - om3 - om4 },
            {    -19, 2*l4 - om3 - psi },
            {    -48, 2*(l4 - om4) },
            {    167, 2*l4 - om4 - psi },
            {   -142, 2*(l4 - psi) },
            {    148, l3 - 2*l4 + pi4 },
            {    -94, l3 - 2*l4 + pi3 },
            {   -390, l3 - l4 },
            {      9, 2*l3 - 4*l4 + 2*pi4 },
            {    -37, 2*l3 - 3*l4 + pi4 },
            {      6, 2*l3 - 3*l4 + pi3 },
            {   -195, 2*(l3 - l4) },
            {      6, 3*l3 - 7*l4 + 2*pi4 + om4 + psi },
            {    187, 3*l3 - 7*l4 + 4*pi4 },
            {   -149, 3*l3 - 7*l4 + pi3 + 3*pi4 },
            {     51, 3*l3 - 7*l4 + 2*(pi3 + pi4) },
            {    -10, 3*l3 - 7*l4 + 3*pi3 + pi4 },
            {      6, 3*(l3 - 2*l4 + pi4) },
            {     -8, 3*l3 - 4*l4 + pi4 },
            {    -41, 3*(l3 - l4) },
            {    -13, 4*(l3 - l4) },
            {    -44, l2 - 3*l3 + 2*l4 },
            {     89, l2 - l4 },
            {    106, l1 - l4 }
        };

        for (int i = 0; i < NU; i++)
            upsilon += U[i][0] * sin(U[i][1]);
        upsilon *= 1e-7;

        // now use the "time completed" series
        const double n = 21.57107117668 * deg_to_rad;
        computeArguments(t + upsilon/n, l1, l2, l3, l4,
                         om1, om2, om3, om4, psi, Gp, G);

        const int NZ = 18;
        double Z[NZ][2] = {
            {      8, l4 - 2*PIj - om4 - 2*psi },
            {      8, l4 - 2*PIj + psi - 4*G },
            {     88, l4 - 2*PIj + psi - 3*G },
            {    773, l4 - 2*PIj + psi - 2*G },
            {    -38, l4 - 2*PIj + psi - G },
            {      5, l4 - 2*PIj + psi },
            {      9, l4 - om1 },
            {    -17, l4 - om2 },
            {  -5112, l4 - om3 },
            {     -7, l4 - om4 - G },
            {  44134, l4 - om4 },
            {      7, l4 - om4 + G },
            {   -102, l4 - psi - G },
            { -76579, l4 - psi },
            {    104,  l4 - psi + G },
            {    -10, l4 - psi + 5*Gp - 2*G + phi2 },
            {    -11, l3 - 2*l4 + psi },
            {      7, l3 - 2*l4 + om4 }
        };

        for (int i = 0; i < NZ; i++)
            zeta += Z[i][0] * sin(Z[i][1]);
        zeta *= 1e-7;
    }
    break;
    default:
        xpExit("Unknown Jupiter satellite\n", __FILE__, __LINE__);
    }

    // Jupiter equatorial coordinates
    X = radius * cos(lon - psi + upsilon);
    Y = radius * sin(lon - psi + upsilon);
    Z = radius * zeta;

    // rotate to Jupiter's orbital plane
    const double I = 3.10401 * deg_to_rad;
    rotateX(X, Y, Z, -I);

    // rotate towards ascending node of Jupiter's equator on its
    // orbital plane
    const double OM = 99.95326 * deg_to_rad;
    rotateZ(X, Y, Z, OM - psi);

    // rotate to ecliptic
    const double J = 1.30691 * deg_to_rad;
    rotateX(X, Y, Z, -J);

    // rotate towards ascending node of Jupiter's orbit on ecliptic
    rotateZ(X, Y, Z, -OM);

    // rotate to earth equator B1950
    const double eps = 23.4457889 * deg_to_rad;
    rotateX(X, Y, Z, -eps);

    // precess to J2000
    precessB1950J2000(X, Y, Z);
}
