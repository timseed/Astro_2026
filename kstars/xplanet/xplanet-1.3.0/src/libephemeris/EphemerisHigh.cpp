#include "jpleph.h"

#include "xpUtil.h"

#include "EphemerisHigh.h"

#include <sstream>
using namespace std;

EphemerisHigh::EphemerisHigh(const string &ephemerisFile) : Ephemeris()
{
    ephem_ = jpl_init_ephemeris(ephemerisFile.c_str(), NULL, NULL);

    if (ephem_ == NULL)
    {
        ostringstream errMsg;
        errMsg << "Can't initialize ephemeris from " 
               << ephemerisFile << "\n";
        xpExit(errMsg.str(), __FILE__, __LINE__);
    }
}

EphemerisHigh::~EphemerisHigh()
{
    jpl_close_ephemeris(ephem_);
}

void
EphemerisHigh::GetHeliocentricXYZ(const body b, const double tjd, 
                                  double &Px, double &Py, double &Pz)
{
    int target = -1;
    switch (b)
    {
    case MERCURY:
        target = 1; 
        break;
    case VENUS:
        target = 2; 
        break;
    case EARTH:
        target = 3;
        break;
    case MARS:
        target = 4;
        break;
    case JUPITER:
        target = 5;
        break;
    case SATURN:
        target = 6;
        break;
    case URANUS:
        target = 7;
        break;
    case NEPTUNE:
        target = 8;
        break;
    case PLUTO:
        target = 9;
        break;
    case MOON:
        target = 10;
        break;
    case SUN:
        Px = 0;
        Py = 0;
        Pz = 0;
        target = 11;
        return;
        break;
    default:
        xpExit("Invalid body in ephemeris()\n", __FILE__, __LINE__);
        break;
    }    

    jpl_get_long(ephem_, JPL_EPHEM_N_CONSTANTS);
    const double start = jpl_get_double(ephem_, JPL_EPHEM_START_JD);
    const double end = jpl_get_double(ephem_, JPL_EPHEM_END_JD);

    if (tjd < start || tjd > end)
    {
        ostringstream errMsg;
        errMsg << "Date (" << fromJulian(tjd) << ") out of range of file ("
               << fromJulian(start) << " to " << fromJulian(end) << ")\n";
        xpExit(errMsg.str(), __FILE__, __LINE__);
    }

    double r[6] = { 0, 0, 0, 0, 0, 0 };
    const int origin = 11; // 11 = sun, 12 = solar system barycenter
    const int calcVelocity = 0; // calculates velocities if nonzero

    if (jpl_pleph(ephem_, tjd, target, origin, r, calcVelocity))
        xpWarn("Error in jpl_pleph\n", __FILE__, __LINE__);
    
    Px = r[0];
    Py = r[1];
    Pz = r[2];

    double Vx = 0, Vy = 0, Vz = 0;
    Vx = r[3];
    Vy = r[4];
    Vz = r[5];
}
