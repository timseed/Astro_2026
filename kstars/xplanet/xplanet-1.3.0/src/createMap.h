#ifndef CREATEMAP_H
#define CREATEMAP_H

#include <map>

class Map;
class Planet;
class PlanetProperties;
class Ring;

extern Map *
createMap(const double sLat, const double sLon, 
	  const double obsLat, const double obsLon, 
	  const int width, const int height, 
	  const double pR, Planet *p, Ring *r, 
	  std::map<double, Planet *> &planetsFromSunMap,
	  PlanetProperties *planetProperties);

#endif
