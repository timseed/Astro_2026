#include <sstream>
using namespace std;

#include "config.h"
#include "body.h"
#include "keywords.h"
#include "xpUtil.h"

#include "libannotate/libannotate.h"
#include "libplanet/Planet.h"

void
findBodyXYZ(const double julianDay, const body bodyIndex, const int bodyID, 
            double &X, double &Y, double &Z)
{
    switch (bodyIndex)
    {
    case NAIF:
    {
#ifdef HAVE_CSPICE
        Planet Sun(julianDay, SUN);
        Sun.calcHeliocentricEquatorial();
        calculateSpicePosition(julianDay, bodyID, &Sun, 10, 
                               X, Y, Z);
#else
        ostringstream errStr;
        errStr << "Xplanet was compiled without SPICE support.\n";
        xpWarn(errStr.str(), __FILE__, __LINE__);
#endif
    }
    break;
    case NORAD:
    {
        const time_t tv_sec = get_tv_sec(julianDay);
        double lat, lon, rad;
        if (calculateSatellitePosition(tv_sec, bodyID, lat, lon, rad))
        {
            Planet earth(julianDay, EARTH);
            earth.calcHeliocentricEquatorial();
            earth.PlanetographicToXYZ(X, Y, Z, lat, lon, rad);
        }
        else
        {
            ostringstream errStr;
            errStr << "Can't get position of satellite # " << bodyID
                   << ".\n";
            xpWarn(errStr.str(), __FILE__, __LINE__);
        }
    }
    break;
    default:
    {
        Planet t(julianDay, bodyIndex);
        t.calcHeliocentricEquatorial(); 
        t.getPosition(X, Y, Z);
    }
    }
}

void
findBodyVelocity(const double julianDay, 
                 const body bodyIndex, const int bodyID, 
                 const body relativeIndex, const int relativeID, 
                 double &vX, double &vY, double &vZ)
{
    const double delT = 0.01;

    double bmX, bmY, bmZ;
    double bpX, bpY, bpZ;
    findBodyXYZ(julianDay - delT/2, bodyIndex, bodyID,
                bmX, bmY, bmZ);
    findBodyXYZ(julianDay + delT/2, bodyIndex, bodyID,
                bpX, bpY, bpZ);

    double rmX, rmY, rmZ;
    double rpX, rpY, rpZ;
    findBodyXYZ(julianDay - delT/2, relativeIndex, relativeID,
                rmX, rmY, rmZ);
    findBodyXYZ(julianDay + delT/2, relativeIndex, relativeID,
                rpX, rpY, rpZ);

    vX = ((bpX - rpX) - (bmX - rmX)) / delT;
    vY = ((bpY - rpY) - (bmY - rmY)) / delT;
    vZ = ((bpZ - rpZ) - (bmZ - rmZ)) / delT;
}
