#ifndef SETPOSITIONS_H
#define SETPOSITIONS_H

#include <map>
#include <vector>

struct LBRPoint;

class Planet;
class PlanetProperties;

extern void
setOriginXYZFromFile(const std::vector<LBRPoint> &originVector, 
                     const std::vector<LBRPoint>::iterator &iterOriginVector);

extern void
setTargetXYZ(PlanetProperties *planetProperties[]);

extern void
setOriginXYZ(PlanetProperties *planetProperties[]);

extern void
getObsLatLon(Planet *target, PlanetProperties *planetProperties[]);

extern void
setUpXYZ(const Planet *target, std::map<double, Planet *> &planetsFromSunMap,
         double &upX, double &upY, double &upZ);

#endif
