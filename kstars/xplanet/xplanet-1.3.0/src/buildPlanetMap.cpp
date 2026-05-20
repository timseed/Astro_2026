#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
using namespace std;

#include "body.h"
#include "buildPlanetMap.h"
#include "Options.h"
#include "xpUtil.h"

#include "libplanet/Planet.h"

static Planet *p[RANDOM_BODY];
static bool firstTime = true;

Planet *
findPlanetinMap(map<double, Planet *> &planetMap, body b)
{
    Planet *returnVal = NULL;

    for (map<double, Planet *>::iterator it0 = planetMap.begin();
         it0 != planetMap.end(); it0++)
    {
        Planet *p = it0->second;
        if (p->Index() == b) 
        {
            returnVal = p;
            break;
        }
    }
    return(returnVal);
}

void
buildPlanetMap(const double jd, map<double, Planet *> &planetMap)
{
    buildPlanetMap(jd, 0, 0, 0, false, planetMap);
}

void
buildPlanetMap(const double jd, 
               const double oX, const double oY, const double oZ, 
               const bool light_time, map<double, Planet *> &planetMap)
{
    planetMap.clear();
    if (!firstTime) destroyPlanetMap();
    firstTime = false;

    Options *options = Options::getInstance();
    if (options->PrintEphemeris())
    {
        printf("%10s: %12s %13s %13s %13s %13s\n", 
               "Name", "Julian Day", 
               "X", "Y", "Z", "R");
    }

    for (int ibody = SUN; ibody < RANDOM_BODY; ibody++)
    {
        body b = (body) ibody;

        // Compute the planet's position
        double pX, pY, pZ;
        p[ibody] = new Planet(jd, b);
        p[ibody]->calcHeliocentricEquatorial(true);
        p[ibody]->getPosition(pX, pY, pZ);

        // Now get the position relative to the origin
        double dX = pX - oX;
        double dY = pY - oY;
        double dZ = pZ - oZ;
        double dist = sqrt(dX*dX + dY*dY + dZ*dZ);

        // Account for light time, if desired
        if (light_time)
        {
            double lt = dist * AU_to_km / 299792.458;
            lt /= 86400;
            delete p[ibody];

            p[ibody] = new Planet(jd - lt, b);
            p[ibody]->calcHeliocentricEquatorial();
            p[ibody]->getPosition(pX, pY, pZ);
        }

        // Now store this body in the map, using the heliocentric
        // distance as the key
        dist = sqrt(pX*pX + pY*pY + pZ*pZ);
        planetMap.insert(make_pair(dist, p[ibody]));

        if (options->PrintEphemeris())
        {
            printf("%10s: %12.4f %13.9f %13.9f %13.9f %13.9f\n", 
                   body_string[ibody], jd, pX, pY, pZ, dist);
        }
    }
    if (options->PrintEphemeris())
    {
        const double dist = sqrt(oX*oX + oY*oY + oZ*oZ);
        printf("%10s: %12.4f %13.9f %13.9f %13.9f %13.9f\n", 
               "origin", jd, oX, oY, oZ, dist);
        exit(EXIT_SUCCESS);
    }

}

void
destroyPlanetMap()
{
    for (int ibody = SUN; ibody < RANDOM_BODY; ibody++) 
    {
        delete p[ibody];
        p[ibody] = NULL;
    }
}
