#include <cmath>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <sstream>
#include <string>
using namespace std;

#include "findFile.h"
#include "Options.h"
#include "Planet.h"
#include "xpUtil.h"

#include "libephemeris/ephemerisWrapper.h"

body
Planet::parseBodyName(char *name)
{
    body return_body;
    char *lowercase = name;
    char *ptr = name;
    while (*ptr != '\0') *ptr++ = tolower(*name++);
    
    if (strncmp(lowercase, "above", 2) == 0)
        return_body = ABOVE_ORBIT;
    else if (strncmp(lowercase, body_string[ARIEL], 2) == 0)
        return_body = ARIEL;
    else if (strncmp(lowercase, "below", 1) == 0)
        return_body = BELOW_ORBIT;
    else if (strncmp(lowercase, body_string[CALLISTO], 2) == 0)
        return_body = CALLISTO;
    else if (strncmp(lowercase, body_string[CHARON], 2) == 0)
        return_body = CHARON;
    else if (strncmp(lowercase, "default", 3) == 0)
        return_body = DEFAULT;
    else if (strncmp(lowercase, body_string[DEIMOS], 3) == 0)
        return_body = DEIMOS;
    else if (strncmp(lowercase, body_string[DIONE], 2) == 0)
        return_body = DIONE;
    else if (strncmp(lowercase, body_string[EARTH], 2) == 0)
        return_body = EARTH;
    else if (strncmp(lowercase, body_string[ENCELADUS], 2) == 0)
        return_body = ENCELADUS;
    else if (strncmp(lowercase, body_string[EUROPA], 2) == 0)
        return_body = EUROPA;
    else if (strncmp(lowercase, body_string[GANYMEDE], 1) == 0)
        return_body = GANYMEDE;
    else if (strncmp(lowercase, body_string[HYPERION], 1) == 0)
        return_body = HYPERION;
    else if (strncmp(lowercase, body_string[IAPETUS], 2) == 0)
        return_body = IAPETUS;
    else if (strncmp(lowercase, body_string[IO], 2) == 0)
        return_body = IO;
    else if (strncmp(lowercase, body_string[JUPITER], 1) == 0)
        return_body = JUPITER;
    else if (strncmp(lowercase, "major", 3) == 0)
        return_body = MAJOR_PLANET;
    else if (strncmp(lowercase, body_string[MARS], 3) == 0)
        return_body = MARS;
    else if (strncmp(lowercase, body_string[MERCURY], 2) == 0)
        return_body = MERCURY;
    else if (strncmp(lowercase, body_string[MIMAS], 3) == 0)
        return_body = MIMAS;
    else if (strncmp(lowercase, body_string[MIRANDA], 3) == 0)
        return_body = MIRANDA;
    else if (strncmp(lowercase, body_string[MOON], 2) == 0)
        return_body = MOON;
    else if (strncmp(lowercase, "naif", 2) == 0)
        return_body = NAIF;
    else if (strncmp(lowercase, body_string[NEPTUNE], 3) == 0)
        return_body = NEPTUNE;
    else if (strncmp(lowercase, body_string[NEREID], 3) == 0)
        return_body = NEREID;
    else if (strncmp(lowercase, "norad", 2) == 0)
        return_body = NORAD;
    else if (strncmp(lowercase, body_string[OBERON], 1) == 0)
        return_body = OBERON;
    else if (strncmp(lowercase, "path", 2) == 0)
        return_body = ALONG_PATH;
    else if (strncmp(lowercase, body_string[PHOBOS], 4) == 0)
        return_body = PHOBOS;
    else if (strncmp(lowercase, body_string[PHOEBE], 4) == 0)
        return_body = PHOEBE;
    else if (strncmp(lowercase, body_string[PLUTO], 2) == 0)
        return_body = PLUTO;
    else if (strncmp(lowercase, "random", 2) == 0)
        return_body = RANDOM_BODY;
    else if (strncmp(lowercase, body_string[RHEA], 2) == 0)
        return_body = RHEA;
    else if (strncmp(lowercase, body_string[SATURN], 2) == 0)
        return_body = SATURN;
    else if (strncmp(lowercase, body_string[SUN], 2) == 0)
        return_body = SUN;
    else if (strncmp(lowercase, "system", 2) == 0)
        return_body = SAME_SYSTEM;
    else if (strncmp(lowercase, body_string[TETHYS], 2) == 0)
        return_body = TETHYS;
    else if (strncmp(lowercase, body_string[TITANIA], 6) == 0)
        return_body = TITANIA;
    else if (strncmp(lowercase, body_string[TITAN], 5) == 0)
        return_body = TITAN;
    else if (strncmp(lowercase, body_string[TRITON], 2) == 0)
        return_body = TRITON;
    else if (strncmp(lowercase, body_string[URANUS], 2) == 0)
        return_body = URANUS;
    else if (strncmp(lowercase, body_string[UMBRIEL], 2) == 0)
        return_body = UMBRIEL;
    else if (strncmp(lowercase, body_string[VENUS], 1) == 0)
        return_body = VENUS;
    else
    {
        xpWarn("parseBody: Unknown body specified\n", 
               __FILE__, __LINE__);
        return_body = UNKNOWN_BODY;
    }
    return(return_body);
}

/*
  Rotational parameters are from Seidelmann et al. (2002), Celestial
  Mechanics 82, 83--110.
*/
Planet::Planet(const double jd, const body this_body) 
    : index_(this_body), 
      julianDay_(jd), 
      needRotationMatrix_(true), 
      period_(0),
      needShadowCoeffs_(true)
{
    Options *options = Options::getInstance();
    if (options->UniversalTime())
        julianDay_ += delT(julianDay_) / 86400;

    d2000_ = (julianDay_ - 2451545.0);
    T2000_ = d2000_ / 36525;

    switch (index_)
    {
    case SUN:
        primary_ = SUN;

        alpha0_ = 286.13;
        delta0_ = 63.87;

        nullMeridian0_ = 84.10;
        wdot_ = 14.1844;

        flipped_ = 1;

        period_ = 0;
        radiusEq_ = 696000;
        flattening_ = 0;

        break;
    case MERCURY:
        primary_ = SUN;

        alpha0_ = 281.01 - 0.033 * T2000_;
        delta0_ = 61.45 - 0.005 * T2000_;

        nullMeridian0_ = 329.548;
        wdot_ = 6.1385025;

        flipped_ = -1; 

        period_ = 87.969;

        radiusEq_ = 2439;
        flattening_ = 0;

        break;
    case VENUS:
        primary_ = SUN;

        alpha0_ = 272.76;
        delta0_ = 67.16;

        nullMeridian0_ = 160.20;
        wdot_ = -1.4813688;

        flipped_ = 1; 

        period_ = 224.701;

        radiusEq_ = 6051.9;
        flattening_ = 0;

        break;
    case EARTH:
        primary_ = SUN;
        
        alpha0_ = 0 - 0.641 * T2000_;
        delta0_ = 90 - .557 * T2000_;
        
        nullMeridian0_ = 190.147;
        wdot_ = 360.9856235;
        
        flipped_ = 1;

        period_ = 365.256;
        
        // WGS84 ellipsoid
        radiusEq_ = 6378.137;
        flattening_ = 1/298.257223563; 
        
        break;
    case MOON:
    {
        primary_ = EARTH;

        const double E1 = (125.045 -  0.0529921 * d2000_) * deg_to_rad;
        const double E2 = (250.089 -  0.1059842 * d2000_) * deg_to_rad;
        const double E3 = (260.008 + 13.0120009 * d2000_) * deg_to_rad;
        const double E4 = (176.625 + 13.3407154 * d2000_) * deg_to_rad;
        const double E5 = (357.529 +  0.9856003 * d2000_) * deg_to_rad;
        const double E6 = (311.589 + 26.4057084 * d2000_) * deg_to_rad;
        const double E7 = (134.963 + 13.0649930 * d2000_) * deg_to_rad;
        const double E8 = (276.617 +  0.3287146 * d2000_) * deg_to_rad;
        const double E9 = ( 34.226 +  1.7484877 * d2000_) * deg_to_rad;
        const double E10 = ( 15.134 -  0.1589763 * d2000_) * deg_to_rad;
        const double E11 = (119.743 +  0.0036096 * d2000_) * deg_to_rad;
        const double E12 = (239.961 +  0.1643573 * d2000_) * deg_to_rad;
        const double E13 = ( 25.053 + 12.9590088 * d2000_) * deg_to_rad;

        alpha0_ = 269.9949 + (0.0031 * T2000_        - 3.8787 * sin(E1)
                              - 0.1204 * sin(E2) + 0.0700 * sin(E3)
                              - 0.0172 * sin(E4) + 0.0072 * sin(E6)
                              - 0.0052 * sin(E10) + 0.0043 * sin(E13));
        delta0_ =  66.5392 + (0.0130 * T2000_        + 1.5419 * cos(E1)
                              + 0.0239 * cos(E2) - 0.0278 * cos(E3)
                              + 0.0068 * cos(E4) - 0.0029 * cos(E6)
                              + 0.0009 * cos(E7) + 0.0008 * cos(E10)
                              - 0.0009 * cos(E13));
        nullMeridian0_ = 38.3213 + (d2000_ * (13.17635815 - 1.4E-12 * d2000_) 
                                    + 3.5610 * sin(E1) 
                                    + 0.1208 * sin(E2)
                                    - 0.0642 * sin(E3)
                                    + 0.0158 * sin(E4)
                                    + 0.0252 * sin(E5)
                                    - 0.0066 * sin(E6)
                                    - 0.0047 * sin(E7)
                                    - 0.0046 * sin(E8)
                                    + 0.0028 * sin(E9)
                                    + 0.0052 * sin(E10)
                                    + 0.0040 * sin(E11)
                                    + 0.0019 * sin(E12)
                                    - 0.0044 * sin(E13));
        wdot_ = 0;

        flipped_ = 1;
        
        period_ = 27.321661;

        radiusEq_ = 1737.5;
        flattening_ = 0.002;

    }
    break;
    case MARS:
    case PHOBOS:
    case DEIMOS:
    {
        flipped_ = -1;

        const double M1 = (169.51 - 0.4357640 * d2000_) * deg_to_rad;
        const double M2 = (192.93 + 1128.4096700 * d2000_) * deg_to_rad;
        const double M3 = (53.47 - 0.0181510 * d2000_) * deg_to_rad;

        switch (index_)
        {
        case MARS:
            primary_ = SUN;
            
            alpha0_ = 317.68143 - 0.1061 * T2000_;
            delta0_ = 52.88650 - 0.0609 * T2000_;
            
            nullMeridian0_ = 176.630;
            wdot_ = 350.89198226;
            
            period_ = 686.980;
            
            radiusEq_ = 3397;
            flattening_ = 0.0065;

            break;
        case PHOBOS:
            primary_ = MARS;

            alpha0_ = 317.68 - 0.108 * T2000_ + 1.79 * sin(M1);
            delta0_ = 52.90 - 0.061 * T2000_ - 1.08 * cos(M1);

            nullMeridian0_ = (35.06 + 8.864 * T2000_ * T2000_ - 1.42 * sin(M1) 
                              - 0.78 * sin(M2));
            wdot_ = 1128.8445850;

            period_ = 0.31891023;

            radiusEq_ = 10;  // non-spherical
            flattening_ = 0;

            break;
        case DEIMOS:
            primary_ = MARS;

            alpha0_ = 316.65 - 0.108 * T2000_ + 2.98 * sin(M3);
            delta0_ = 53.52 - 0.061 * T2000_ - 1.78 * cos(M3);

            nullMeridian0_ = (79.41 - 0.520 * T2000_ * T2000_ 
                              - 2.58 * sin(M3) + 0.19 * cos(M3));
            wdot_ = 285.1618970;

            period_ = 1.2624407;
            radiusEq_ = 6;  // non-spherical
            flattening_ = 0;

            break;
        default:
            break;
        }
    }
    break;
    case JUPITER:
    case IO:
    case EUROPA:
    case GANYMEDE:
    case CALLISTO:
    {
        flipped_ = -1;

        const double J3 = (283.90 +  4850.7 * T2000_) * deg_to_rad;
        const double J4 = (355.80 +  1191.3 * T2000_) * deg_to_rad;
        const double J5 = (119.90 +   262.1 * T2000_) * deg_to_rad;
        const double J6 = (229.80 +    64.3 * T2000_) * deg_to_rad;
        const double J7 = (352.25 +  2382.6 * T2000_) * deg_to_rad;
        const double J8 = (113.35 +  6070.0 * T2000_) * deg_to_rad;

        switch (index_)
        {
        case JUPITER:
            primary_ = SUN;

            alpha0_ = 268.05 - 0.009 * T2000_;
            delta0_ = 64.49 + 0.003 * T2000_;

            // System III (magnetic field rotation)
            nullMeridian0_ = 284.95;
            wdot_ = 870.5366420;

            if (options->GRSSet())
            {
                // System II
                nullMeridian0_ = 43.3;
                wdot_ = 870.270;
            }

            period_ = 4332.589;

            radiusEq_ = 71492;
            flattening_ = 0.06487;

            break;
        case IO:
            primary_ = JUPITER;
            
            alpha0_ = (268.05 - 0.009 * T2000_ + 0.094 * sin(J3) 
                       + 0.024 * sin(J4));
            delta0_ = (64.50 + 0.003 * T2000_ + 0.040 * cos(J3)
                       + 0.011 * cos(J4));
            nullMeridian0_ = 200.39 - 0.085 * sin(J3) - 0.022 * sin(J4);
            wdot_ = 203.4889538;

            period_ = 1.769137786;

            radiusEq_ = 1821.6;
            flattening_ = 0;

            break;
        case EUROPA:
            primary_ = JUPITER;
            
            alpha0_ = (268.08 - 0.009 * T2000_ + 1.086 * sin(J4)
                       + 0.060 * sin(J5) + 0.015 * sin(J6)
                       + 0.009 * sin(J7));
            delta0_ = (64.51 + 0.003 * T2000_ + 0.468 * cos(J4)
                       + 0.026 * cos(J5) + 0.007 * cos(J6)
                       + 0.002 * cos(J7));
            nullMeridian0_ = (35.67 - 0.980 * sin(J4) - 0.054 * sin(J5)
                              - 0.014 * sin(J6) - 0.008 * sin(J7));
            wdot_ = 101.3747235;

            period_ = 3.551181041;

            radiusEq_ = 1560.8;
            flattening_ = 0;

            break;
        case GANYMEDE:
            primary_ = JUPITER;
            
            alpha0_ = (268.20 - 0.009 * T2000_ - 0.037 * sin(J4) 
                       + 0.431 * sin(J5) + 0.091 * sin(J6));
            delta0_ = (64.57 + 0.003 * T2000_ - 0.016 * cos(J4)
                       + 0.186 * cos(J5) + 0.039 * cos(J6));
            nullMeridian0_ = (44.04 + 0.033 * sin(J4) - 0.389 * sin(J5)
                              - 0.082 * sin(J6));
            wdot_ = 50.3176081;

            period_ = 7.15455296;

            radiusEq_ = 2631.2;
            flattening_ = 0;
 
            break;
        case CALLISTO:
            primary_ = JUPITER;
            
            alpha0_ = (268.72 - 0.009 * T2000_ - 0.068 * sin(J5)
                       + 0.590 * sin(J6) + 0.010 * sin(J8));
            delta0_ = (64.83 + 0.003 * T2000_ - 0.029 * cos(J5)
                       + 0.254 * cos(J6) - 0.004 * cos(J8));
            nullMeridian0_ = (259.73 + 0.061 * sin(J5) - 0.533 * sin(J6)
                              - 0.009 * sin(J8));
            wdot_ = 21.5710715;

            period_ = 16.6890184;

            radiusEq_ = 2410.3; 
            flattening_ = 0;

            break;
        default:
            break;
        }
    }
    break;
    case SATURN:
    case MIMAS:
    case ENCELADUS:
    case TETHYS:
    case DIONE:
    case RHEA:
    case TITAN:
    case HYPERION:
    case IAPETUS:
    case PHOEBE:
    {
        flipped_ = -1;

        const double S3 = (177.40 - 36505.5 * T2000_) * deg_to_rad;
        const double S4 = (300.00 -  7225.9 * T2000_) * deg_to_rad;
        const double S5 = (316.45 +   506.2 * T2000_) * deg_to_rad;
        const double S6 = (345.20 -  1016.3 * T2000_) * deg_to_rad;
        const double S7 = ( 29.80 -    52.1 * T2000_) * deg_to_rad;
        switch (index_)
        {
        case SATURN:
            primary_ = SUN;

            alpha0_ = 40.589 - 0.036 * T2000_;
            delta0_ = 83.537 - 0.004 * T2000_;
            nullMeridian0_ = 38.90;
            wdot_ = 810.7939024;

            period_ = 10759.22;

            radiusEq_ = 60268;
            flattening_ = 0.09796;

            break;
        case MIMAS:
            primary_ = SATURN;

            alpha0_ = 40.66 - 0.036 * T2000_ + 13.56 * sin(S3);
            delta0_ = 83.52 - 0.004 * T2000_ - 1.53 * cos(S3);
            nullMeridian0_ = 337.46 - 13.48 * sin(S3) - 44.85 * sin(S5);
            wdot_ = 381.994550;

            period_ = 0.942421813;
            
            radiusEq_ = 198.6;
            flattening_ = 0;

            break;
        case ENCELADUS:
            primary_ = SATURN;

            alpha0_ = 40.66 - 0.036 * T2000_;
            delta0_ = 83.52 - 0.004 * T2000_;

            nullMeridian0_ = 2.82;
            wdot_ = 262.7318996;

            period_ = 1.370217855;

            radiusEq_ = 249.4;
            flattening_ = 0;

            break;
        case TETHYS:
            primary_ = SATURN;

            alpha0_ = 40.66 - 0.036 * T2000_ + 9.66 * sin(S4);
            delta0_ = 83.52 - 0.004 * T2000_ - 1.09 * cos(S4);

            nullMeridian0_ = 10.45 - 9.60 * sin(S4) + 2.23 * sin(S5);
            wdot_ = 190.6979085;

            period_ = 1.887802160;

            radiusEq_ = 529.8;
            flattening_ = 0;

            break;
        case DIONE:
            primary_ = SATURN;

            alpha0_ = 40.66 - 0.036 * T2000_;
            delta0_ = 83.52 - 0.004 * T2000_;

            nullMeridian0_ = 357.00;
            wdot_ = 131.5349316;

            period_ = 2.736914742;

            radiusEq_ = 559;
            flattening_ = 0;

            break;
        case RHEA:
            primary_ = SATURN;

            alpha0_ = 40.38 - 0.036 * T2000_ + 3.10 * sin(S6);
            delta0_ = 83.55 - 0.004 * T2000_ - 0.35 * cos(S6);
            nullMeridian0_ = 235.16 - 3.08 * sin(S6);
            wdot_ = 79.6900478;

            period_ = 4.517500436;

            radiusEq_ = 764;
            flattening_ = 0;

            break;
        case TITAN:
            primary_ = SATURN;

            alpha0_ = 36.41 - 0.036 * T2000_ + 2.66 * sin(S7);
            delta0_ = 83.94 - 0.004 * T2000_ - 0.30 * cos(S7);
            nullMeridian0_ = 189.64 - 2.64 * sin(S7);
            wdot_ = 22.5769768;

            period_ = 15.94542068;

            radiusEq_ = 2575;
            flattening_ = 0;

            break;
        case HYPERION:
            primary_ = SATURN;

            alpha0_ = 40.589 - 0.036 * T2000_;
            delta0_ = 83.537 - 0.004 * T2000_;

            nullMeridian0_ = 0;
            wdot_ = 0;

            period_ = 21.2766088;

            radiusEq_ = 141.5;   // non-spherical
            flattening_ = 0;

            break;
        case IAPETUS:
            primary_ = SATURN;

            alpha0_ = 318.16 - 3.949 * T2000_;
            delta0_ = 75.03 - 1.143 * T2000_;
            nullMeridian0_ = 350.20;
            wdot_ = 4.5379572;

            period_ = 79.3301825;

            radiusEq_ = 718;
            flattening_ = 0;

            break;
        case PHOEBE:
            primary_ = SATURN;
            
            alpha0_ = 355.00;
            delta0_ = 68.70;

            nullMeridian0_ = 304.70;
            wdot_ = 930.8338720;

            period_ = 550.48;

            radiusEq_ = 110;
            flattening_ = 0;

            break;
        default:
            break;
        }
    }
    break;
    case URANUS:
    case MIRANDA:
    case ARIEL:
    case UMBRIEL:
    case TITANIA:
    case OBERON:
    {
        flipped_ = 1;

        const double U11 = (141.69 + 41887.66 * T2000_) * deg_to_rad;
        const double U12 = (316.41 + 2863.96 * T2000_) * deg_to_rad;
        const double U13 = (304.01 - 51.94 * T2000_) * deg_to_rad;
        const double U14 = (308.71 - 93.17 * T2000_) * deg_to_rad;
        const double U15 = (340.82 - 75.32 * T2000_) * deg_to_rad;
        const double U16 = (259.14 - 504.81 * T2000_) * deg_to_rad;

        switch (index_)
        {
        case URANUS:
            primary_ = SUN; 
            
            alpha0_ = 257.311;
            delta0_ = -15.175;
            nullMeridian0_ = 203.81;
            wdot_ = -501.1600928;

            period_ = 30685.4;
            
            radiusEq_ = 25559;
            flattening_ = 0.02293;

            break;
        case MIRANDA:
            primary_ = URANUS;

            alpha0_ = 257.42 + 4.41 * sin(U11) - 0.04 * sin(2*U11);
            delta0_ = -15.08 + 4.25 * cos(U11) - 0.02 * cos(2*U11);
            nullMeridian0_ = (30.70 - 1.27 * sin(U12) + 0.15 * sin(2*U12)
                              + 1.15 * sin(U11) - 0.09 * sin(2*U11));
            wdot_ = -254.6906892;

            period_ = 1.41347925;

            radiusEq_ = 235.8;
            flattening_ = 0;

            break;
        case ARIEL:
            primary_ = URANUS;

            alpha0_ = 257.43 + 0.29 * sin(U13);
            delta0_ = -15.10 + 0.28 * cos(U13);
            nullMeridian0_ = 156.22 + 0.05 * sin(U12) + 0.08 * sin(U13);
            wdot_ = -142.8356681;

            period_ = 2.52037935;

            radiusEq_ = 578.9;
            flattening_ = 0;

            break;
        case UMBRIEL:
            primary_ = URANUS;

            alpha0_ = 257.43 + 0.21 * sin(U14);
            delta0_ = -15.10 + 0.20 * cos(U14);
            nullMeridian0_ = 108.05 - 0.09 * sin(U12) + 0.06 * sin(U14);
            wdot_ = -86.8688923;

            period_ = 4.1441772;

            radiusEq_ = 584.7;
            flattening_ = 0;

            break;
        case TITANIA:
            primary_ = URANUS;

            alpha0_ = 257.43 + 0.29 * sin(U15);
            delta0_ = -15.10 + 0.28 * cos(U15);
            nullMeridian0_ = 77.74 + 0.08 * sin(U15);
            wdot_ = -41.3514316;

            period_ = 8.7058717;

            radiusEq_ = 788.9;
            flattening_ = 0;

            break;
        case OBERON:
            primary_ = URANUS;

            alpha0_ = 257.43 + 0.16 * sin(U16);
            delta0_ = -15.10 + 0.16 * cos(U16);
            nullMeridian0_ = 6.77 + 0.04 * sin(U16);
            wdot_ = -26.7394932;

            period_ = 13.4632389;

            radiusEq_ = 761.4;
            flattening_ = 0;

            break;
        default:
            break;
        }
    }
    break;
    case NEPTUNE:
    case TRITON:
    case NEREID:
    {
        flipped_ = -1;

        switch (index_)
        {
        case NEPTUNE:
        {
            primary_ = SUN; 
            
            double N = (357.85 + 52.316 * T2000_) * deg_to_rad;
            alpha0_ = 299.36 + 0.70 * sin(N);
            delta0_ = 43.46 - 0.51 * cos(N);
            
            nullMeridian0_ = 253.18 - 0.48 * sin(N);
            wdot_ = 536.3128492;

            period_ = 60189.0;
            
            radiusEq_ = 24764;
            flattening_ = 0.0171;

        }
        break;
        case TRITON:
        {
            primary_ = NEPTUNE;

            const double N7 = (177.85 + 52.316 * T2000_) * deg_to_rad;
            alpha0_ = (299.36 - 32.35 * sin(N7) - 6.28 * sin(2*N7)
                       - 2.08 * sin(3*N7) - 0.74 * sin(4*N7) 
                       - 0.28 * sin(5*N7) - 0.11 * sin(6*N7)
                       - 0.07 * sin(7*N7) - 0.02 * sin(8*N7)
                       - 0.01 * sin(9*N7));
            delta0_ = (43.46 + 22.55 * cos(N7) + 2.10 * cos(2*N7)
                       + 0.55 * cos(3*N7) + 0.16 * cos(4*N7) 
                       + 0.05 * cos(5*N7) + 0.02 * cos(6*N7)
                       + 0.01 * cos(7*N7));
            
            nullMeridian0_ = (296.53 + 22.25 * sin(N7) + 6.73 * sin(2*N7)
                              + 2.05 * sin(3*N7) + 0.74 * sin(4*N7)
                              + 0.28 * sin(5*N7) + 0.11 * sin(6*N7) 
                              + 0.05 * sin(7*N7) + 0.02 * sin(8*N7)
                              + 0.01 * sin(9*N7));

            wdot_ = -61.2572637;

            period_ = 5.8768541;

            radiusEq_ = 1353;
            flattening_ = 0;
        }
        break;
        case NEREID:
        {
            primary_ = NEPTUNE;

            alpha0_ = 299.36;
            delta0_ = 43.46;
            
            nullMeridian0_ = 0;
            wdot_ = 0;

            period_ = 360.13619;

            radiusEq_ = 170;
            flattening_ = 0;
        }
        break;
        default:
            break;
        }
    }
    break;
    case PLUTO:
    case CHARON:
    {
        flipped_ = 1;
        
        alpha0_ = 313.02;
        delta0_ = 9.09;
        
        wdot_ = -56.3623195;
        
        switch (index_)
        {
        case PLUTO:
            primary_ = SUN; 
            
            nullMeridian0_ = 236.77;
            
            period_ = 90465.0;
            
            radiusEq_ = 1151;
            flattening_ = 0;
            break;
        case CHARON:
            primary_ = PLUTO;

            nullMeridian0_ = 56.77;

            period_ = 6.38723;

            radiusEq_ = 593;
            flattening_ = 0;
            break;
        default:
            break;
        }
    }
    break;
    default:
        xpExit("Planet: Unknown body specified.\n", __FILE__, __LINE__);
    }

    alpha0_ *= deg_to_rad;
    delta0_ *= deg_to_rad;

    radiusEq_ /= AU_to_km;

    nullMeridian_ = fmod(nullMeridian0_ + wdot_ * d2000_, 360.0);
    nullMeridian_ *= deg_to_rad;

    radiusPol_ = radiusEq_ * (1 - flattening_);
    omf2_ = (1 - flattening_) * (1 - flattening_);
}

Planet::~Planet()
{
}

// Return the distance from the surface to the center, in units of
// equatorial radius.  Input latitude is assumed to be planetographic.
double
Planet::Radius(const double lat) const
{
    double returnValue = 1;
    if (index_ == JUPITER ||
        index_ == SATURN)
    {
        double tmpLat = lat;
        double tmpLon = 0;
        PlanetographicToPlanetocentric(tmpLat, tmpLon);
        
        const double ReSinLat = radiusEq_  * sin(tmpLat);
        const double RpCosLat = radiusPol_ * cos(tmpLat);
        
        // from http://mathworld.wolfram.com/Ellipse.html
        returnValue = radiusPol_ / sqrt(RpCosLat * RpCosLat 
                                        + ReSinLat * ReSinLat);
    }

    return(returnValue);
}

// Compute heliocentric equatorial coordinates of the planet
void
Planet::calcHeliocentricEquatorial()
{
    calcHeliocentricEquatorial(true);
}

void
Planet::calcHeliocentricEquatorial(const bool relativeToSun)
{
    GetHeliocentricXYZ(index_, primary_, julianDay_, 
                       relativeToSun, X_, Y_, Z_);
}

// Return rectangular coordinates in heliocentric equatorial frame
void
Planet::getPosition(double &X, double &Y, double &Z) const
{
    X = X_;
    Y = Y_;
    Z = Z_;
}

double
Planet::Illumination(const double oX, const double oY, const double oZ)
{
    return(50 * (ndot(X_, Y_, Z_, X_ - oX, Y_ - oY, Z_ - oZ) + 1));
}

void
Planet::CreateRotationMatrix()
{
    /* 
       These equations are from "Astronomy on the Personal Computer,
       3rd Edition", by Montenbruck and Pfleger, Chapter 7.
    */
    const double cosW = cos(nullMeridian_);
    const double sinW = sin(nullMeridian_);

    const double cosA = cos(alpha0_);
    const double sinA = sin(alpha0_);

    const double cosD = cos(delta0_);
    const double sinD = sin(delta0_);

    rot_[0][0] = -cosW * sinA - sinW * sinD * cosA;
    rot_[0][1] =  cosW * cosA - sinW * sinD * sinA;
    rot_[0][2] =  sinW * cosD;

    rot_[1][0] =  sinW * sinA - cosW * sinD * cosA;
    rot_[1][1] = -sinW * cosA - cosW * sinD * sinA;
    rot_[1][2] =  cosW * cosD;

    rot_[2][0] =  cosD * cosA;
    rot_[2][1] =  cosD * sinA;
    rot_[2][2] =  sinD;

    invertMatrix(rot_, invRot_);

    needRotationMatrix_ = false;
}

void
Planet::PlanetocentricToXYZ(double &X, double &Y, double &Z,
                            const double lat, const double lon, 
                            const double rad)
{
    if (needRotationMatrix_) CreateRotationMatrix();

    double r[3];
    r[0] = cos(lat) * cos(lon);
    r[1] = cos(lat) * sin(lon);
    r[2] = sin(lat);

    double newrad = rad * radiusEq_;
    X = dot(invRot_[0], r) * newrad;
    Y = dot(invRot_[1], r) * newrad;
    Z = dot(invRot_[2], r) * newrad;

    X += X_;
    Y += Y_;
    Z += Z_;
}

void
Planet::PlanetographicToXYZ(double &X, double &Y, double &Z,
                            double lat, double lon, const double rad)
{
    PlanetographicToPlanetocentric(lat, lon);
    PlanetocentricToXYZ(X, Y, Z, lat, lon, rad);
}

void
Planet::XYZToPlanetocentric(const double X, const double Y, const double Z,
                            double &lat, double &lon)
{
    double rad = 0;
    XYZToPlanetocentric(X, Y, Z, lat, lon, rad);
}

void
Planet::XYZToPlanetocentric(const double X, const double Y, const double Z,
                            double &lat, double &lon, double &rad)
{
    if (needRotationMatrix_) CreateRotationMatrix();

    const double r[3] = { X - X_, Y - Y_, Z - Z_ };

    const double sx = dot(rot_[0], r);
    const double sy = dot(rot_[1], r);
    const double sz = dot(rot_[2], r);

    rad = sqrt(sx * sx + sy*sy);
    if (rad > 0)
        lat = atan(sz/rad);
    else
        lat = 0;

    if (cos(lat) > 0)
        lon = atan2(sy, sx);
    else
        lon = 0;

    if (lon < 0) lon += TWO_PI;

    rad = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    rad /= radiusEq_;
}

void
Planet::XYZToPlanetographic(const double X, const double Y, const double Z,
                            double &lat, double &lon)
{
    double rad = 0;
    XYZToPlanetographic(X, Y, Z, lat, lon, rad);
}

void
Planet::XYZToPlanetographic(const double X, const double Y, const double Z,
                            double &lat, double &lon, double &rad)
{
    XYZToPlanetocentric(X, Y, Z, lat, lon, rad);
    PlanetocentricToPlanetographic(lat, lon);
}

void
Planet::XYZToPlanetaryXYZ(const double X, const double Y, const double Z,
                          double &pX, double &pY, double &pZ)
{
    double lat, lon, rad;
    XYZToPlanetocentric(X, Y, Z, lat, lon, rad);

    pX = rad * cos(lat) * cos(lon);
    pY = rad * cos(lat) * sin(lon);
    pZ = rad * sin(lat);
}

void
Planet::PlanetaryXYZToXYZ(const double pX, const double pY, const double pZ,
                          double &X, double &Y, double &Z)
{
    const double lat = atan(pZ / sqrt(pX * pX + pY * pY));
    double lon;
    if (cos(lat) > 1e-5)
        lon = atan2(pY, pX);
    else
        lon = 0;

    const double rad = sqrt(pX * pX + pY * pY + pZ * pZ);

    PlanetocentricToXYZ(X, Y, Z, lat, lon, rad);
}

void
Planet::PlanetocentricToPlanetographic(double &lat, double &lon) const
{
    if (flattening_ > 0)
        lat = atan(tan(lat) / omf2_);

    if (index_ == EARTH || index_ == SUN) lon *= -1;
    if (wdot_ > 0) lon *= -1;
    if (lon < 0) lon += TWO_PI;
}

void
Planet::PlanetographicToPlanetocentric(double &lat, double &lon) const
{
    if (flattening_ > 0)
        lat = atan(omf2_ * tan(lat));

    if (index_ == EARTH || index_ == SUN) lon *= -1;
    if (wdot_ > 0) lon *= -1;
}

void
Planet::ComputeShadowCoeffs()
{
    XYZToPlanetaryXYZ(0, 0, 0, sunX_, sunY_, sunZ_);

    ellipseCoeffC_ = sunZ_ * sunZ_ / omf2_;
    ellipseCoeffC_ += sunY_ * sunY_;
    ellipseCoeffC_ += sunX_ * sunX_;
    ellipseCoeffC_ -= 1;

    needShadowCoeffs_ = false;
}

// x, y, z must be in planetary XYZ frame
bool
Planet::IsInMyShadow(const double x, const double y, const double z)
{
    if (needShadowCoeffs_) ComputeShadowCoeffs();

    double ellipseCoeffA = (z - sunZ_) * (z - sunZ_) / omf2_;
    ellipseCoeffA += (y - sunY_) * (y - sunY_);
    ellipseCoeffA += (x - sunX_) * (x - sunX_);

    double ellipseCoeffB = (z - sunZ_) * sunZ_ / omf2_;
    ellipseCoeffB += (y - sunY_) * sunY_;
    ellipseCoeffB += (x - sunX_) * sunX_;

    const double determinant = (ellipseCoeffB * ellipseCoeffB 
                                - ellipseCoeffA * ellipseCoeffC_);

    return(determinant > 0);
}

void
Planet::getOrbitalNorth(double &X, double &Y, double &Z) const
{
    if (index_ == SUN)
    {
        X = cos(delta0_) * cos(alpha0_);
        Y = cos(delta0_) * sin(alpha0_);
        Z = sin(delta0_);
        return;
    }

    // cross product of position and velocity vectors points to the
    // orbital north pole
    double pos[3] = { X_, Y_, Z_ };

    double pX0, pY0, pZ0;
    double pX1, pY1, pZ1;

    GetHeliocentricXYZ(index_, primary_, julianDay_ - 0.5, 
                       true, pX0, pY0, pZ0);
    GetHeliocentricXYZ(index_, primary_, julianDay_ + 0.5, 
                       true, pX1, pY1, pZ1);

    double vel[3] = { pX1 - pX0, pY1 - pY0, pZ1 - pZ0 };

    double north[3];
    cross(pos, vel, north);

    double mag = sqrt(dot(north, north));
    
    X = north[0]/mag;
    Y = north[1]/mag;
    Z = north[2]/mag;
}

void
Planet::getBodyNorth(double &X, double &Y, double &Z) const
{
    // get direction of the rotational axis
    X = cos(alpha0_) * cos(delta0_);
    Y = sin(alpha0_) * cos(delta0_);
    Z = sin(delta0_);
}
