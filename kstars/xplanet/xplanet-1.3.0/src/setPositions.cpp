#include <cstdlib>
#include <cstring>
#include <map>
#include <sstream>
using namespace std;

#include "buildPlanetMap.h"
#include "findBodyXYZ.h"
#include "keywords.h"
#include "PlanetProperties.h"
#include "Options.h"
#include "readOriginFile.h"
#include "Separation.h"
#include "xpUtil.h"

#include "libplanet/Planet.h"

// if an origin file is being used, set the observer position from it
static void
setOriginXYZFromFile(const vector<LBRPoint> &originVector, 
                     const vector<LBRPoint>::iterator &iterOriginVector)
{
    Options *options = Options::getInstance();

    // if an origin file is specified, set observer position from it
    if (!options->OriginFile().empty())
    {
        if (options->InterpolateOriginFile())
        {
            // interpolate positions in the origin file to the
            // current time
            double thisRad, thisLat, thisLon, thisLocalTime = -1;
            interpolateOriginFile(options->JulianDay(), 
                                  originVector, thisRad, 
                                  thisLat, thisLon, thisLocalTime);
            options->Range(thisRad);
            options->Latitude(thisLat);
            options->Longitude(thisLon);
            options->LocalTime(thisLocalTime);
        }
        else
        {
            // Use the next time in the origin file
            options->setTime(iterOriginVector->time);

            // Use the position, if specified, in the origin file
            if (iterOriginVector->radius > 0)
            {
                options->Range(iterOriginVector->radius);
                options->Latitude(iterOriginVector->latitude);
                options->Longitude(iterOriginVector->longitude);

                // Use the local time, if specified, in the origin file
                if (iterOriginVector->localTime > 0)
                    options->LocalTime(iterOriginVector->localTime);
            }
        }
    }

    // if -dynamic_origin was specified, set observer position
    // from it.
    if (!options->DynamicOrigin().empty())
    {
        LBRPoint originPoint;
        readDynamicOrigin(options->DynamicOrigin(), originPoint);
        options->Range(originPoint.radius);
        options->Latitude(originPoint.latitude);
        options->Longitude(originPoint.longitude);
        options->LocalTime(originPoint.localTime);
    }
}

// set the position of the target.  The coordinates are stored in the
// Options class.
static void
setTargetXYZ(PlanetProperties *planetProperties[])
{
    Options *options = Options::getInstance();

    switch (options->TargetMode())
    {
    case MAJOR:
    case RANDOM:
    {
        bool no_random_target = true;
        for (int i = 0; i < RANDOM_BODY; i++)
        {
            if (planetProperties[i]->RandomTarget()) 
            {
                no_random_target = false;
                break;
            }
        }

        if (no_random_target)
        {
            ostringstream errMsg;
            errMsg << "random_target is false for all bodies.  "
                   << "Check your config file.\n";
            xpExit(errMsg.str(), __FILE__, __LINE__);
        }

        bool found_target = false;
        while (!found_target)
        {
            body target = (body) (random() % RANDOM_BODY);

            options->Target(target);

            found_target = planetProperties[target]->RandomTarget();

            if (found_target && options->TargetMode() == MAJOR)
            {
                Planet p(options->JulianDay(), options->Target());
                found_target = (p.Primary() == SUN);
            }
        }
    }
    // fall through
    case BODY:
    {
        Planet t(options->JulianDay(), options->Target());
        options->Primary(t.Primary());
    }
    break;
    case XYZ:
        if (options->Target() == NORAD)
        {
            options->Primary(EARTH);
        }
        else
        {
            options->Primary(SUN);
        }
        break;
    default:
        xpExit("Unknown target mode?\n", __FILE__, __LINE__);
    }

    double tX, tY, tZ;
    if (options->Target() == ALONG_PATH)
    {
        findBodyVelocity(options->JulianDay(), options->Origin(), 
                         options->OriginID(), options->PathRelativeTo(), 
                         options->PathRelativeToID(), 
                         tX, tY, tZ);
        
        tX *= FAR_DISTANCE;
        tY *= FAR_DISTANCE;
        tZ *= FAR_DISTANCE;
    }
    else
    {
        findBodyXYZ(options->JulianDay(), options->Target(), 
                    options->TargetID(), tX, tY, tZ);
    }
    options->setTarget(tX, tY, tZ);
}

// set the position of the origin.  The coordinates are stored in the
// Options class.
static void
setOriginXYZ(PlanetProperties *planetProperties[])
{
    double oX, oY, oZ;

    Options *options = Options::getInstance();
    switch (options->OriginMode())
    {
    case LBR:
    {
        if (options->RandomLatLonRot())
        {
            double longitude;
            longitude =  random() % 360;
            longitude *= deg_to_rad;
            options->Longitude(longitude);

            // Weight random latitudes towards the equator
            double latitude; 
            latitude = (random() % 2000)/1000.0 - 1;
            latitude = asin(latitude);
            options->Latitude(latitude);

            double rotate0;
            rotate0 = random() % 360;
            rotate0 *= deg_to_rad;
            options->Rotate0(rotate0);
            options->Rotate(0);
        }
        
        if (options->Origin() == NAIF || options->Origin() == NORAD)
        {
            findBodyXYZ(options->JulianDay(), options->Origin(),    
                        options->OriginID(), oX, oY, oZ);
        }
        else
        {
            // Set the observer's lat & lon.  Usually this is in the
            // frame of the target body, but if the origin file option
            // is used, the observer is in the origin body's
            // geographic frame.
            body referenceBody = options->Target();
            if (!options->OriginFile().empty()
                || !options->DynamicOrigin().empty()) 
                referenceBody = options->Origin();
            
            Planet p(options->JulianDay(), referenceBody);
            p.calcHeliocentricEquatorial(); 
            
            if (options->LocalTime() >= 0)
            {
                double subSolarLat = 0;
                double subSolarLon = 0;
                p.XYZToPlanetographic(0, 0, 0, subSolarLat, subSolarLon);

                double longitude;
                longitude = (subSolarLon - M_PI
                             + p.Flipped() * options->LocalTime() * M_PI / 12);
                options->Longitude(longitude);
            }
            p.PlanetographicToXYZ(oX, oY, oZ,
                                  options->Latitude(), options->Longitude(), 
                                  options->Range());
        }
    }
    break;
    case MAJOR:
    case RANDOM:
    case SYSTEM:
    {
        // check to see that at least one body has random_origin=true
        bool no_random_origin = true;
        for (int i = 0; i < RANDOM_BODY; i++)
        {
            if (i == options->Target()) continue;

            if (planetProperties[i]->RandomOrigin()) 
            {
                no_random_origin = false;
                break;
            }
        }

        if (no_random_origin)
        {
            ostringstream errMsg;
            errMsg << "Target is " 
                   << planetProperties[options->Target()]->Name()
                   << ", random_origin is false for all other bodies.  "
                   << "Check your config file.\n";
            xpExit(errMsg.str(), __FILE__, __LINE__);
        }

        bool found_origin = false;
        while (!found_origin)
        {
            options->Origin(static_cast<body> (random() % RANDOM_BODY));

            if (options->Verbosity() > 1)
            {
                ostringstream msg;
                msg << "target = " << body_string[options->Target()] 
                    << ", origin = " << body_string[options->Origin()] << endl;
                xpMsg(msg.str(), __FILE__, __LINE__);
            }

            if (options->Target() == options->Origin()) continue;
            
            if (options->OriginMode() == RANDOM)
            {
                found_origin = true;
            }
            else
            {
                Planet o(options->JulianDay(), options->Origin());
                
                if (options->OriginMode() == MAJOR)
                {
                    found_origin = (o.Primary() == SUN);
                }
                else if (options->OriginMode() == SYSTEM)
                {
                    // SYSTEM means one of three things:
                    // 1) origin is target's primary
                    // 2) target is origin's primary
                    // 3) target and origin have same primary
                    found_origin = ((options->Origin() == options->Primary()
                                     || o.Primary() == options->Target()
                                     || options->Primary() == o.Primary()));
                }
            }

            if (found_origin) 
                found_origin = planetProperties[options->Origin()]->RandomOrigin();
        } // while (!found_origin)
    }
    // fall through
    case BODY:
    {
        findBodyXYZ(options->JulianDay(), options->Origin(),    
                    options->OriginID(), oX, oY, oZ);
        
        if (options->OppositeSide())
        {
            double tX, tY, tZ;
            options->getTarget(tX, tY, tZ);
            oX = 2*tX - oX;
            oY = 2*tY - oY;
            oZ = 2*tZ - oZ;
        }
    }
    break;
    case ABOVE:
    case BELOW:
    {
        if (options->Target() == SUN)
        {
            ostringstream errMsg;
            errMsg << "-origin above or below not applicable for SUN. "
                   << "Use -latitude 90 or -90 instead\n";
            xpExit(errMsg.str(), __FILE__, __LINE__);
        }

        double pX, pY, pZ;
        findBodyXYZ(options->JulianDay(), options->Primary(), -1, pX, pY, pZ);

        double vX, vY, vZ;
        findBodyVelocity(options->JulianDay(), options->Target(), 
                         options->TargetID(), options->Primary(), -1, 
                         vX, vY, vZ);

        // cross product of position and velocity vectors points to the
        // orbital north pole
        double tX, tY, tZ;
        options->getTarget(tX, tY, tZ);
        double pos[3] = { tX - pX, tY - pY, tZ - pZ };
        double vel[3] = { vX, vY, vZ };

        double north[3];
        cross(pos, vel, north);

        double mag = sqrt(dot(north, north));
        Planet primary(options->JulianDay(), options->Primary());
        double dist = FAR_DISTANCE * primary.Radius();
        
        oX = dist * north[0]/mag + pX;
        oY = dist * north[1]/mag + pY;
        oZ = dist * north[2]/mag + pZ;

        const double radius = sqrt(pos[0] * pos[0] + pos[1] * pos[1] 
                                   + pos[2] * pos[2]);

        // will only be zero if -fov or -radius haven't been specified
        if (options->FieldOfView() < 0)
        {
            options->FieldOfView(4*radius/dist); // fit the orbit on the screen
            options->FOVMode(FOV);
        }

        if (options->OriginMode() == BELOW)
        {
            oX -= 2 * (oX - pX);
            oY -= 2 * (oY - pX);
            oZ -= 2 * (oZ - pX);
        }
    }
    break;
    }

    if (options->RangeSpecified() && options->TargetMode() != XYZ)
    {
        Planet p2(options->JulianDay(), options->Target());
        p2.calcHeliocentricEquatorial(); 

        double latitude, longitude;
        p2.XYZToPlanetographic(oX, oY, oZ, latitude, longitude);
        p2.PlanetographicToXYZ(oX, oY, oZ, latitude, longitude, 
                               options->Range());
    }

    if (options->SeparationTarget() < RANDOM_BODY) 
    {
        // rectangular coordinates of the separation target
        double sX, sY, sZ;
        findBodyXYZ(options->JulianDay(), options->SeparationTarget(), -1, 
                    sX, sY, sZ);

        // rectangular coordinates of the primary target
        double tX, tY, tZ;
        Planet p(options->JulianDay(), options->Target());
        p.calcHeliocentricEquatorial();
        p.getPosition(tX, tY, tZ);

        // Save the range
        double latitude, longitude, range;
        p.XYZToPlanetographic(oX, oY, oZ, latitude, longitude, range);

        // place the observer 90 degrees from the separation target in
        // the primary target's equatorial plane
        double n[3];
        p.getBodyNorth(n[0], n[1], n[2]);

        double s[3] = { sX - tX, sY - tY, sZ - tZ };
        double c[3];
        cross(s, n, c);
        c[0] += tX;
        c[1] += tY;
        c[2] += tZ;

        p.XYZToPlanetographic(c[0], c[1], c[2], latitude, longitude);
        p.PlanetographicToXYZ(oX, oY, oZ, latitude, longitude, range);

        Separation sep(oX, oY, oZ, tX, tY, tZ, sX, sY, sZ);
        sep.setSeparation(options->SeparationDist() * deg_to_rad);
        sep.getOrigin(oX, oY, oZ);
    }

    options->setOrigin(oX, oY, oZ);
}

// Set the observer's lat/lon with respect to the target body, if
// appropriate
void
getObsLatLon(Planet *target, PlanetProperties *planetProperties[])
{
    Options *options = Options::getInstance();

    // XYZ mode is where the target isn't a planetary body.
    // (e.g. the Cassini spacecraft)
    if (options->TargetMode() == XYZ)
    {
        if (options->LightTime()) setTargetXYZ(planetProperties);
    }
    else
    {
        if (target == NULL)
            xpExit("Can't find target body?\n", __FILE__, __LINE__);

        if (options->LightTime())
        {
            double tX, tY, tZ;
            target->getPosition(tX, tY, tZ);
            options->setTarget(tX, tY, tZ);
        }

        // Rectangular coordinates of the observer
        double oX, oY, oZ;
        options->getOrigin(oX, oY, oZ);

        // Find the sub-observer lat & lon
        double obs_lat, obs_lon;
        target->XYZToPlanetographic(oX, oY, oZ, obs_lat, obs_lon);
        options->Latitude(obs_lat);
        options->Longitude(obs_lon);

        // Find the sub-solar lat & lon.  This is used for the image
        // label
        double sunLat, sunLon;
        target->XYZToPlanetographic(0, 0, 0, sunLat, sunLon);
        options->SunLat(sunLat);
        options->SunLon(sunLon);
    }
}

// Set the direction of the "Up" vector
void
setUpXYZ(const Planet *target, map<double, Planet *> &planetsFromSunMap,
         double &upX, double &upY, double &upZ)
{
    Options *options = Options::getInstance();

    switch (options->North())
    {
    default:
        xpWarn("Unknown value for north, using body\n", 
               __FILE__, __LINE__);
    case BODY:
        target->getBodyNorth(upX, upY, upZ);
        break;
    case GALACTIC:
    {
        const double GN_LAT = 27.4 * deg_to_rad;
        const double GN_LON = 192.25 * deg_to_rad;
        
        upX = cos(GN_LAT) * cos(GN_LON);
        upY = cos(GN_LAT) * sin(GN_LON);
        upZ = sin(GN_LAT);
    }
    break;
    case ORBIT:
    {
        // if it's a moon, pretend its orbital north is the same as
        // its primary, although in most cases it's the same as its
        // primary's rotational north
        if (target->Primary() == SUN)
        {
            target->getOrbitalNorth(upX, upY, upZ);
        }
        else
        {
            Planet *primary = findPlanetinMap(planetsFromSunMap, 
                                              target->Primary());
            primary->getOrbitalNorth(upX, upY, upZ);
        }
    }
    break;
    case PATH:
    {
        double vX, vY, vZ;
        findBodyVelocity(options->JulianDay(), 
                         options->Origin(), 
                         options->OriginID(),
                         options->PathRelativeTo(), 
                         options->PathRelativeToID(), 
                         vX, vY, vZ);
            
        upX = vX;
        upY = vY;
        upZ = vZ;
    }
    break;
    case SEPARATION:
    {
        if (options->SeparationTarget() < RANDOM_BODY)
        {
            double sX, sY, sZ;
            double tX, tY, tZ;
            double oX, oY, oZ;
            
            findBodyXYZ(options->JulianDay(), options->SeparationTarget(), 
                        options->TargetID(), sX, sY, sZ);
            options->getOrigin(oX, oY, oZ);
            options->getTarget(tX, tY, tZ);
            
            double t[3] = {tX - oX, tY - oY, tZ - oZ};
            double s[3] = {sX - oX, sY - oY, sZ - oZ};
            double c[3];
            cross(s, t, c);
            
            if (sin(options->SeparationDist() * deg_to_rad) > 0)
            {
                upX = c[0];
                upY = c[1];
                upZ = c[2];
            }
            else
            {
                upX = -c[0];
                upY = -c[1];
                upZ = -c[2];
            }
        }
        else
        {
            xpWarn("No separation target given, using -north body\n", 
                   __FILE__, __LINE__);
            target->getBodyNorth(upX, upY, upZ);
        }
    }
    break;
    case TERRESTRIAL:
        upX = 0;
        upY = 0;
        upZ = 1;
        break;
    }

    double up[3] = {upX, upY, upZ};
    normalize(up);
    upX = up[0];
    upY = up[1];
    upZ = up[2];
}

void 
setPositions(const vector<LBRPoint> &originVector, 
             const vector<LBRPoint>::iterator &iterOriginVector,
             Planet *&target, map<double, Planet *> &planetMap,
             PlanetProperties *planetProperties[])
{
    Options *options = Options::getInstance();

    // set the observer's XYZ coordinates if an origin file has
    // been specified
    setOriginXYZFromFile(originVector, iterOriginVector);

    // Set the target's XYZ coordinates.
    setTargetXYZ(planetProperties);

    // Set the origin's XYZ coordinates.
    setOriginXYZ(planetProperties);

    // Find the target body in the list
    body target_body = options->Target();
    target = findPlanetinMap(planetMap, target_body);

    if (options->LightTime())
    {
        double oX, oY, oZ;
        options->getOrigin(oX, oY, oZ);

        // Destroy the existing planet map and build a new one
        buildPlanetMap(options->JulianDay(), oX, oY, oZ, true, 
                       planetMap);

        target = findPlanetinMap(planetMap, target_body);
            
        // Set the target's XYZ coordinates.
        setTargetXYZ(planetProperties);
            
        // Set the origin's XYZ coordinates.
        setOriginXYZ(planetProperties);
    }

    // Now find the observer's lat/lon
    getObsLatLon(target, planetProperties);
}
